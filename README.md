*This project has been created as part of the 42 curriculum by jzorreta*

## Description

**Codexion** is a concurrency simulation built for the 42 curriculum. It models a circular co-working hub where **coders** (threads) compete for **quantum USB dongles** (shared resources), in the spirit of Dijkstra’s dining philosophers.

Coders repeatedly cycle through three phases: **compiling**, **debugging**, and **refactoring**. Compiling requires **two dongles at once** (left and right). The goal is to coordinate access with **POSIX mutexes and condition variables**, optional **FIFO** or **EDF** scheduling on each dongle, and a **monitor thread** that detects **burnout** when a coder waits too long without **starting** a new compile.

---

## Ring topology: coders and dongles

The subject describes an **inclusive circular hub**: **N coders** and **N dongles**. Each coder has a **left** dongle and a **right** dongle. Physically, every dongle sits **between two coders**—it is the right dongle of one seat and the left dongle of the next seat going clockwise. Coder **1** is adjacent to coder **N**; that is why the “right” dongle of coder *i* is indexed with modulo arithmetic: the last coder’s right dongle is the same object as the first coder’s left dongle.

### Table-top view (linear unwrap, same ring)

Imagine walking clockwise around the table. You alternate **coder → dongle → coder → dongle → …** until you close the loop:

```text
  Coder 1       Dongle 0       Coder 2       Dongle 1       Coder 3   …   Coder N       Dongle N-1
     o──────────────●──────────────o──────────────●──────────────o  ~  o──────────────●────────────── |
     ^                                                                                                |
     └──────────────────────────── closes back to Coder 1 ────────────────────────────────────────────┘
```

Each **dongle d** (with `d` from `0` to `N - 1`) sits between two seats: it is the **left** dongle of **coder d + 1** and the **right** dongle of **coder d**, where **coder 0** in that formula means **coder N** so that **dongle 0** lies between **coder N** and **coder 1**. Compiling always needs **two adjacent dongles** at once—the two on either side of that coder’s chair.

### Tiny example for N = 4

```text
        +---------+     +---------+     +---------+     +---------+
        | Coder 1 |     | Coder 2 |     | Coder 3 |     | Coder 4 |
        +----+----+     +----+----+     +----+----+     +----+----+
    	|      \           /      \     /         \     /         |
        |       [D0]     [D1]       [D2]            [D3]          |
        |                                        				  |  
    	+---------------------------------------------------------+
```

- **Coder 1** needs dongles **0** (left) and **1** (right).  
- **Coder 2** needs **1** and **2**.  
- **Coder 4** needs **3** and **0**—the wrap-around edge is where dining-philosopher-style deadlocks appear if everyone grabs resources in an inconsistent order.

This project therefore acquires the two required dongles in a **global numeric order** whenever they are distinct, so the wait-for graph stays acyclic.

---

## Instructions

### Compilation

The root `Makefile` builds with `cc`, `-Wall -Wextra -Werror`, and `-pthread`.

```bash
make
```

### Execution

Exactly **eight** arguments are required (all integers except the scheduler name):

```bash
./codexion <nb_coders> <t_burnout> <t_compile> <t_debug> <t_refactor> <nb_compiles> <cooldown> <scheduler>
```

| Argument | Description |
| --- | --- |
| `nb_coders` | Number of coders and number of dongles on the table. |
| `t_burnout` | Milliseconds without **starting** a compile since the last compile start (or since simulation start) before burnout. |
| `t_compile` | Milliseconds spent compiling while holding both dongles. |
| `t_debug` | Milliseconds spent debugging (after both dongles are released). |
| `t_refactor` | Milliseconds spent refactoring; then the coder tries to compile again. |
| `nb_compiles` | If **every** coder has compiled at least this many times, the simulation stops successfully. |
| `cooldown` | After a dongle is released, it cannot be taken again until this many milliseconds have passed. |
| `scheduler` | Must be exactly `fifo` or `edf` (case-sensitive per subject). |

Invalid arguments (non-integers, negative values where disallowed, unknown scheduler) are rejected.

### Example

Four coders, EDF arbitration:

```bash
./codexion 4 1200 200 200 200 5 100 edf
```

---

## FIFO and EDF: what changes, and why it matters

Both policies decide **who gets a dongle next** when several coders want the **same** dongle at the same time. Each dongle has its own wait structure (protected by that dongle’s mutex); the policy only changes **how the next granted waiter is chosen**. Cooldown rules and “you must already be at the head of the queue” logic apply equally to both.

### FIFO (`fifo`) — first in, first out

**Idea:** treat each dongle like a fair ticket counter. When a coder blocks because the dongle is busy, in cooldown, or someone else is ahead in line, their request is stamped with an ordering key that reflects **when they entered the wait set** (in this project, essentially “now” at the moment of `heap_push`).

**Strengths:**

* Predictable: whoever asked first tends to get served first at that dongle, which matches an intuitive notion of **fairness between neighbors**.
* Easy to reason about when reading logs: long monotonic stretches of “arrival order” behavior.

**Weaknesses (conceptual, not bugs):**

* “Fair locally” does not mean “everyone’s burnout clock is equally urgent globally.” A coder who has **plenty of slack** before burnout still queues like everyone else, while a coder who is **about to miss** their deadline might be stuck behind several earlier arrivals on **each** dongle they still need. Under harsh parameters, FIFO can still lose the race against time even though no single dongle queue looks unfair.

**When you might run it:** stable workloads, teaching runs where you want queueing discipline to be obvious, or comparisons against EDF on the same seed/parameters.

### EDF (`edf`) — earliest deadline first

**Idea:** treat each dongle’s waiters as a priority queue ordered by **urgency of burnout**, not by arrival time. The subject defines the relevant deadline as:

`deadline = last_compile_start + time_to_burnout`

The coder whose **deadline is soonest** should be preferred at the head of the line (smaller deadline first in a min-heap). That aligns the dongle scheduler with the same clock the **monitor** uses to declare burnout: you are constantly trying to serve the thread that has the **least slack** left before it becomes invalid to let them **start** another compile.

**Strengths:**

* Matches real-time scheduling intuition: **reduce the risk of missing hard deadlines** by always favoring the most time-pressed competitor at each shared resource.
* Under **feasible** parameters, the subject expects **liveness** (no starvation) with EDF; giving the most urgent waiters first chance at dongles is the standard way to justify that property in academic phrasing.

**Weaknesses / caveats:**

* Slightly harder to read from logs alone, because “who goes next” is no longer “who showed up first.”
* Real clocks are noisy; two coders can get **numerically equal** deadlines. The subject therefore expects a **deterministic tie-break** (here: lower coder id wins) so two runs with the same logical race do not diverge arbitrarily.

**When you might run it:** aggressive `time_to_burnout`, heavy contention, or when you deliberately want the simulation to behave like a soft real-time system protecting the most at-risk thread.

### Side-by-side summary

| | **FIFO** | **EDF** |
| --- | --- | --- |
| **Sort key (this project)** | Time the request entered the dongle’s heap (arrival / “ticket number”). | `last_compile_start + time_to_burnout` (burnout deadline). |
| **Who tends to win under contention?** | Earlier waiters at that dongle. | Waiter with the **soonest** burnout deadline. |
| **Philosophy** | Democracy in the queue. | Triage in the queue. |
| **Subject tie-break** | Implicit in arrival ordering; equal timestamps are rare but handled by heap stability rules. | Equal deadlines → **lower coder id** first (fully deterministic). |

### One paragraph, if you only remember one thing

**FIFO** is the noble “take a number and wait your turn” policy: simple, legible, and locally fair per dongle. **EDF** is the “treat the ER waiting room by severity” policy: it optimizes for not letting anyone’s **burnout clock** expire if the machine can still save them in time. Codexion implements both on top of the **same** mutex, condition variable, and min-heap machinery; only the **priority pushed into the heap** changes.

---

## Output (log format)

Each line is: **elapsed ms since simulation start**, **coder id**, **message**. State changes must not interleave on one line (output is protected by a mutex).

| Message | Meaning |
| --- | --- |
| `X has taken a dongle` | Coder `X` acquired one dongle (two such lines precede compiling). |
| `X is compiling` | `X` started compiling (dongles already held). |
| `X is debugging` | Compile finished; dongles released. |
| `X is refactoring` | Debug phase finished. |
| `X burned out` | Monitor detected that `X` missed the burnout deadline; simulation stops. |

Burnout should be reported within about **10 ms** of the real deadline (monitor polls on a short interval).

---

## When the simulation stops

1. **Success:** every coder has completed at least `nb_compiles` full cycles (each cycle ends after refactoring).  
2. **Burnout:** at least one coder exceeds `t_burnout` without **starting** a new compile; the monitor logs burnout, sets the simulation stopped, and threads exit cleanly.

---

## Edge cases and “impossible” parameters

- With **one coder**, left and right refer to the **same** dongle: holding two distinct dongles is impossible. The coder does not perform a fake compile; the monitor eventually logs **burnout** after `t_burnout`, which matches the subject’s idea that coders need two dongles to compile.
- With **two or more coders**, dongles are acquired in a **fixed global order** (by dongle id) to avoid **circular wait** and deadlock.
- If the simulation is stopped while a coder is waiting on a dongle, waiters are removed from the per-dongle **heap** and partially held dongles are released so shutdown stays consistent.

---

## Program layout

| Path | Role |
| --- | --- |
| `sources/codexion.c` | Entry point, argument parsing, init, launch, cleanup. |
| `sources/coder.c` | Per-coder thread routine (acquire dongles, phases, compile count). |
| `sources/dongle.c` | Mutex/cond around each dongle; FIFO/EDF via heap priorities. |
| `sources/monitor.c` | Burnout checks, “all finished” check, global stop and wakeups. |
| `sources/heap.c` / `sources/scheduler.c` | Min-heap priority queue (mandatory; no STL heap). |
| `sources/init.c` / `sources/cleanup.c` | Lifecycle of mutexes, conds, and allocations. |
| `includes/codexion.h` | Shared types and prototypes. |

---

## Optional testing

The subject encourages local tests (not graded). Examples:

```bash
make && ./codexion 4 1200 200 200 200 3 100 fifo
valgrind --leak-check=full ./codexion 3 2000 100 100 100 2 50 edf
```

Helgrind can be noisy or version-sensitive; if you use it, ensure all mutexes are destroyed only after `pthread_join`, and never unlock a mutex the thread does not hold.

---

## Resources & AI usage

### Technical references

* [POSIX Threads Programming](https://www.geeksforgeeks.org/operating-systems/posix-threads-in-os/)
* [The Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
* [Earliest Deadline First Scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)

### AI usage disclosure

In accordance with Chapter II instructions, AI tools were used to support this project:

* **Concept simplification:** analogies and explanations around synchronization and scheduling.
* **Architecture design:** early structure for the heap-based wait queues and Makefile layout.
* **Debugging:** systematic review of races, deadlocks, cleanup order, and edge cases (e.g. single coder, stop while waiting on a cond).

---

## Blocking cases handled

To keep the simulation **live** and **deterministic** where the subject requires it:

* **Deadlock prevention:** **Circular wait** is broken by acquiring dongles in **strict increasing dongle id order** whenever two distinct dongles are needed (Coffman-style ordering).
* **Starvation and arbitration:**
  * **FIFO:** waiters are ordered by request time (priority = arrival time).
  * **EDF:** waiters are ordered by burnout deadline `last_compile_start + time_to_burnout`; **ties** use a deterministic rule (lower coder id) so ordering is fully defined.
* **Dongle cooldown:** a released dongle stays untakeable until `dongle_cooldown` ms have passed (`released_at` + check under the dongle mutex).
* **Burnout detection:** a dedicated monitor thread compares wall-clock time to each coder’s protected `last_compile_start` and stops the simulation with a serialized log within the required window.
* **Log serialization:** a **log mutex** ensures two messages never share one line; status logging also respects the global stopped flag where appropriate.

---

## Thread synchronization mechanisms

* **`pthread_mutex_t`:** one per **dongle** (state, heap, cond wait), one **`log_mutex`** for `printf`, one **`stop_mutex`** for `stopped` and coordination with `log_status`, and one **`compile_mutex`** per coder for `last_compile_start` shared with the monitor.
* **`pthread_cond_t`:** one per dongle; waiters use **`pthread_cond_timedwait`** so the monitor can stop the simulation and **broadcast** wakes blocked coders without hanging forever.
* **Custom priority queue (min-heap):** encodes the FIFO or EDF policy per dongle without the C standard library’s priority queue; push/pop/sift maintain $O(\log n)$ wait sets.
* **Race prevention:** compile counts and burnout timestamps are read or written only under the correct mutex; the monitor and coders communicate by shared `t_sim` fields and cond broadcasts, not ad hoc flags.
