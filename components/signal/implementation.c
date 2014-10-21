/*| headers |*/

/*| object_like_macros |*/

/*| type_definitions |*/

/*| structure_definitions |*/
struct signal_task {
    {{prefix_type}}SignalSet signals;
};

struct signal {
    struct signal_task tasks[{{tasks.length}}];
};

/*| extern_definitions |*/

/*| function_definitions |*/
static {{prefix_type}}SignalSet _signal_recv({{prefix_type}}SignalSet *cur_task_signals, {{prefix_type}}SignalSet mask);
static void signal_send_set({{prefix_type}}TaskId task_id, {{prefix_type}}SignalSet signals);

/*| state |*/
static struct signal signal_tasks;

/*| function_like_macros |*/
#define _signal_peek(pending_signals, requested_signals) (((pending_signals) & (requested_signals)) != {{prefix_const}}SIGNAL_SET_EMPTY)
#define _signal_pending(task_id, mask) ((PENDING_SIGNALS(task_id) & mask) == mask)
#define PENDING_SIGNALS(task_id) signal_tasks.tasks[task_id].signals

/*| functions |*/
static {{prefix_type}}SignalSet
_signal_recv({{prefix_type}}SignalSet *const pending_signals, const {{prefix_type}}SignalSet requested_signals)
{
    const {{prefix_type}}SignalSet received_signals = *pending_signals & requested_signals;

    precondition_preemption_disabled();

    *pending_signals &= ~received_signals;

    postcondition_preemption_disabled();

    return received_signals;
}

static void
signal_send_set(const {{prefix_type}}TaskId task_id, const {{prefix_type}}SignalSet signals)
{
    precondition_preemption_disabled();

    PENDING_SIGNALS(task_id) |= signals;
    _unblock(task_id);

    postcondition_preemption_disabled();
}

/*| public_functions |*/
{{prefix_type}}SignalSet
{{prefix_func}}signal_wait_set(const {{prefix_type}}SignalSet requested_signals) {{prefix_const}}REENTRANT
{
    {{prefix_type}}SignalSet *const pending_signals = &PENDING_SIGNALS(get_current_task());
    {{prefix_type}}SignalSet received_signals;

    preempt_disable();

    if (_signal_peek(*pending_signals, requested_signals))
    {
        _yield();
    }
    else
    {
        do
        {
            _block();
        } while (!_signal_peek(*pending_signals, requested_signals));
    }

    received_signals = _signal_recv(pending_signals, requested_signals);

    preempt_enable();

    return received_signals;
}

{{prefix_type}}SignalSet
{{prefix_func}}signal_poll_set(const {{prefix_type}}SignalSet requested_signals)
{
    {{prefix_type}}SignalSet *const pending_signals = &PENDING_SIGNALS(get_current_task());
    {{prefix_type}}SignalSet received_signals;

    preempt_disable();

    received_signals = _signal_recv(pending_signals, requested_signals);

    preempt_enable();

    return received_signals;
}

{{prefix_type}}SignalSet
{{prefix_func}}signal_peek_set(const {{prefix_type}}SignalSet requested_signals)
{
    return _signal_peek(PENDING_SIGNALS(get_current_task()), requested_signals);
}

void
{{prefix_func}}signal_send_set(const {{prefix_type}}TaskId task_id, const {{prefix_type}}SignalSet signals)
{
    assert_task_valid(task_id);

    preempt_disable();

    signal_send_set(task_id, signals);

    preempt_enable();
}