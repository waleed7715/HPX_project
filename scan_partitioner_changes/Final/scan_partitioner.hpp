//  Copyright (c) 2007-2025 Hartmut Kaiser
//  Copyright (c)      2015 Daniel Bourgeois
//  Copyright (c)      2017 Taeguk Kwon
//  Copyright (c)      2021 Akhil J Nair
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/assert.hpp>
#include <hpx/modules/async_combinators.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/execution.hpp>
#include <hpx/iterator_support/counting_shape.hpp>
#include <hpx/parallel/util/detail/chunk_size.hpp>
#include <hpx/parallel/util/detail/handle_local_exceptions.hpp>
#include <hpx/parallel/util/detail/scoped_executor_parameters.hpp>
#include <hpx/parallel/util/detail/select_partitioner.hpp>
#include <hpx/topology/topology.hpp>
#include <hpx/modules/coroutines.hpp>
#include <hpx/modules/resource_partitioner.hpp>
#include <hpx/modules/runtime_local.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/modules/async_local.hpp>
#endif

#include <algorithm>
#include <cstddef>
#include <exception>
#include <list>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace hpx::parallel::util {

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        ///////////////////////////////////////////////////////////////////////
        // The static partitioner simply spawns one chunk of iterations for
        // each available core.
        HPX_CXX_CORE_EXPORT template <typename ExPolicy, typename R,
            typename Result1, typename Result2>
        struct scan_static_partitioner
        {
            using parameters_type = typename ExPolicy::executor_parameters_type;
            using executor_type = typename ExPolicy::executor_type;

            using scoped_executor_parameters =
                detail::scoped_executor_parameters_ref<parameters_type,
                    executor_type>;

            using handle_local_exceptions =
                detail::handle_local_exceptions<ExPolicy>;

            template <typename ExPolicy_, typename FwdIter, typename T,
                typename F1, typename F2, typename F3, typename F4>
            static R call([[maybe_unused]] ExPolicy_ policy,
                [[maybe_unused]] FwdIter first,
                [[maybe_unused]] std::size_t count, [[maybe_unused]] T&& init,
                [[maybe_unused]] F1&& f1, [[maybe_unused]] F2&& f2,
                [[maybe_unused]] F3&& f3, [[maybe_unused]] F4&& f4)
            {
#if defined(HPX_COMPUTE_DEVICE_CODE)
                HPX_ASSERT(false);
                return R();
#else
                // inform parameter traits
                scoped_executor_parameters scoped_params(
                    policy.parameters(), policy.executor());

                std::vector<Result1> workitems;
                std::vector<hpx::future<Result2>> finalitems;
                std::list<std::exception_ptr> errors;
                try
                {
                    // pre-initialize first intermediate result
                    workitems.push_back(HPX_FORWARD(T, init));

                    HPX_ASSERT(count > 0);
                    FwdIter first_ = first;
                    std::size_t const count_ = count;

                    // estimate a chunk size based on number of cores used
                    using has_variable_chunk_size = typename hpx::execution::
                        experimental::extract_has_variable_chunk_size<
                            parameters_type>::type;

                    std::size_t cores = 1;
                    auto shape = detail::get_bulk_iteration_shape(
                        has_variable_chunk_size(), policy, workitems, f1, first,
                        count, cores, 1);

                    // schedule every chunk on a separate thread
                    std::size_t size = hpx::util::size(shape);

                    using value_type =
                        typename std::iterator_traits<FwdIter>::value_type;

                    // Computed once per process: number of NUMA domains that
                    // actually have HPX worker threads assigned.  When the
                    // thread count is <= cores per domain (e.g. 12 threads on
                    // a 12-core/NUMA machine) all workers land on NUMA0 and
                    // this returns 1, so the threshold correctly reflects the
                    // full LLC of the single active domain.
                    static std::size_t const num_numa = []() -> std::size_t {
                        auto& topo = hpx::threads::create_topology();
                        if (topo.get_number_of_numa_nodes() <= 1) return 1;

                        auto const& rp = hpx::resource::get_partitioner();
                        std::size_t const num_workers =
                            hpx::get_num_worker_threads();
                        std::size_t active = 0;
                        std::uint64_t seen = 0;
                        for (std::size_t i = 0; i != num_workers; ++i)
                        {
                            std::size_t const domain =
                                topo.get_numa_node_number(rp.get_pu_num(i));
                            std::uint64_t const bit =
                                std::uint64_t(1) << domain;
                            if (!(seen & bit))
                            {
                                seen |= bit;
                                ++active;
                            }
                        }
                        return (active > 0) ? active : 1;
                    }();

                    // Computed once per process: query the LLC size of the machine
                    // at runtime so the threshold adapts without recompilation.
                    // get_cache_size() requires a CPU mask; we use core 0's mask to
                    // get one socket's L3 size, then divide by num_numa (each socket
                    // only sees its own LLC share) and by 2 for input+output
                    // buffers being live simultaneously during the scan.
                    // Falls back to 1M elements if hwloc returns 0.
                    static std::size_t const single_socket_threshold =
                        []() -> std::size_t {
                        auto& topo = hpx::threads::create_topology();
                        auto const mask = topo.get_core_affinity_mask(0);
                        std::size_t const llc_bytes = topo.get_cache_size(mask, 3);
                        std::size_t const fallback = 1'000'000 * sizeof(value_type);
                        std::size_t const effective_llc =
                            (llc_bytes > 0) ? llc_bytes : fallback;
                        return (effective_llc / num_numa / 2) / sizeof(value_type);
                    }();

                    auto const priority_policy =
                        hpx::execution::experimental::with_priority(
                            policy, hpx::threads::thread_priority::bound);

                    auto const hinted_policy =
                        hpx::execution::experimental::with_hint(
                            priority_policy,
                            hpx::threads::thread_schedule_hint{
                                hpx::threads::thread_placement_hint::depth_first});

                    auto const stackless_policy =
                        hpx::execution::experimental::with_stacksize(
                            hinted_policy, hpx::threads::thread_stacksize::nostack);

                    auto const& f1f3_exec =
                        (count_ < single_socket_threshold)
                        ? stackless_policy.executor()
                        : hinted_policy.executor();

                    // If the size of count was enough to warrant testing for a
                    // chunk, pre-initialize second intermediate result and
                    // start f3 for that test chunk immediately (its prefix is
                    // simply init = workitems[0]).  workitems[1] is left as the
                    // raw f1 result so the f2 sequential pass below sees it
                    // exactly once.
                    bool const had_test_chunk = (workitems.size() == 2);
                    if (had_test_chunk)
                    {
                        HPX_ASSERT(count_ > count);

                        workitems.reserve(size + 2);
                        finalitems.reserve(size + 1);

                        finalitems.push_back(
                            execution::async_execute(f1f3_exec,
                                f3, first_, count_ - count, workitems[0]));
                    }
                    else
                    {
                        workitems.reserve(size + 1);
                        finalitems.reserve(size);
                    }

                    {
#if HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
                        static hpx::util::itt::event e("F1_START");
                        hpx::util::itt::event_tick(e);
#endif
                        auto bulk_results =
                            hpx::parallel::execution::bulk_sync_execute(
                                f1f3_exec,
                                [&f1](auto const& elem) {
                                    return HPX_INVOKE(
                                        f1, hpx::get<0>(elem), hpx::get<1>(elem));
                                },
                                shape);

                        for (auto& result : bulk_results)
                            workitems.push_back(HPX_MOVE(result));
                    }

                    // perform f2 sequentially in one go.
                    // When a test chunk was used, workitems[1] already holds the
                    // raw f1 result for that chunk; the loop processes it once
                    // together with the rest, producing correct prefix sums in
                    // workitems[1..N].
                    {
#if HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
                        static hpx::util::itt::event e("F2_START");
                        hpx::util::itt::event_tick(e);
#endif
                        Result1 running = workitems[0];
                        for (std::size_t i = 1; i < workitems.size(); i++)
                        {
                            running = HPX_INVOKE(f2, running, workitems[i]);
                            workitems[i] = running;
                        }
                    }

                    // F3 return type is void.
                    // When a test chunk was used, its f3 was already launched in
                    // finalitems above.  The bulk below covers shape[0..size-1]
                    // which correspond to workitems[f3_offset .. f3_offset+size-1]
                    // (the prefix sums that precede each remaining chunk).
                    {
#if HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
                        static hpx::util::itt::event e("F3_START");
                        hpx::util::itt::event_tick(e);
#endif
                        std::size_t const f3_offset = had_test_chunk ? 1 : 0;
                        hpx::parallel::execution::bulk_sync_execute(
                            f1f3_exec,
                            [f3, workitems, shape = HPX_MOVE(shape), f3_offset](
                                std::size_t idx) {
                                auto it = std::next(
                                    hpx::util::begin(shape), idx);
                                HPX_INVOKE(f3, hpx::get<0>(*it),
                                    hpx::get<1>(*it), workitems[idx + f3_offset]);
                            },
                            hpx::util::counting_shape<std::size_t>(size));
                    }

                    scoped_params.mark_end_of_scheduling();
                }
                catch (...)
                {
                    handle_local_exceptions::call(
                        std::current_exception(), errors);
                }
                return reduce(HPX_MOVE(workitems), HPX_MOVE(finalitems),
                    HPX_MOVE(errors), HPX_FORWARD(F4, f4));
#endif
            }

        private:
            template <typename F>
            static R reduce([[maybe_unused]] std::vector<Result1>&& workitems,
                [[maybe_unused]] std::vector<hpx::future<Result2>>&& finalitems,
                [[maybe_unused]] std::list<std::exception_ptr>&& errors,
                [[maybe_unused]] F&& f)
            {
#if defined(HPX_COMPUTE_DEVICE_CODE)
                HPX_ASSERT(false);
                return R();
#else
                // wait for all tasks to finish
                if (hpx::wait_all_nothrow(finalitems) || !errors.empty())
                {
                    // always rethrow if 'errors' is not empty or 'finalitems'
                    // have an exceptional future
                    handle_local_exceptions::call(finalitems, errors);
                }

                try
                {
                    return f(HPX_MOVE(workitems), HPX_MOVE(finalitems));
                }
                catch (...)
                {
                    // rethrow either bad_alloc or exception_list
                    handle_local_exceptions::call(std::current_exception());
                }

                HPX_UNREACHABLE;    //-V779
#endif
            }
        };

        ///////////////////////////////////////////////////////////////////////
        HPX_CXX_CORE_EXPORT template <typename ExPolicy, typename R,
            typename Result1, typename Result2>
        struct scan_task_static_partitioner
        {
            template <typename ExPolicy_, typename FwdIter, typename T,
                typename F1, typename F2, typename F3, typename F4>
            static hpx::future<R> call(ExPolicy_&& policy, FwdIter first,
                std::size_t count, T&& init, F1&& f1, F2&& f2, F3&& f3,
                F4&& f4)
            {
                return execution::async_execute(policy.executor(),
                    [first, count, policy, init = HPX_FORWARD(T, init),
                        f1 = HPX_FORWARD(F1, f1), f2 = HPX_FORWARD(F2, f2),
                        f3 = HPX_FORWARD(F3, f3),
                        f4 = HPX_FORWARD(F4, f4)]() mutable -> R {
                        using partitioner_type =
                            scan_static_partitioner<ExPolicy, R, Result1,
                                Result2>;
                        return partitioner_type::call(policy, first, count,
                            HPX_MOVE(init), f1, f2, f3, f4);
                    });
            }
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    // ExPolicy:    execution policy
    // R:           overall result type
    // Result1:     intermediate result type of first and second step
    // Result2:     intermediate result of the third step
    HPX_CXX_CORE_EXPORT template <typename ExPolicy, typename R = void,
        typename Result1 = R, typename Result2 = void>
    struct scan_partitioner
      : detail::select_partitioner<std::decay_t<ExPolicy>,
            detail::scan_static_partitioner,
            detail::scan_task_static_partitioner>::template apply<R, Result1,
            Result2>
    {
    };
}    // namespace hpx::parallel::util
