// signal.c - Signal handling
// Like Linux kernel/signal.c

#include <stdint.h>
#include <stdbool.h>

#define NSIG            64
#define _NSIG           NSIG
#define _NSIG_BPW       32
#define _NSIG_WORDS     (_NSIG / _NSIG_BPW)

#define SIGHUP           1
#define SIGINT           2
#define SIGQUIT          3
#define SIGILL           4
#define SIGTRAP          5
#define SIGABRT          6
#define SIGIOT           6
#define SIGBUS           7
#define SIGFPE           8
#define SIGKILL          9
#define SIGUSR1         10
#define SIGSEGV         11
#define SIGUSR2         12
#define SIGPIPE         13
#define SIGALRM         14
#define SIGTERM         15
#define SIGSTKFLT       16
#define SIGCHLD         17
#define SIGCONT         18
#define SIGSTOP         19
#define SIGTSTP         20
#define SIGTTIN         21
#define SIGTTOU         22
#define SIGURG          23
#define SIGXCPU         24
#define SIGXFSZ         25
#define SIGVTALRM       26
#define SIGPROF         27
#define SIGWINCH        28
#define SIGIO           29
#define SIGPOLL         SIGIO
#define SIGPWR          30
#define SIGSYS          31
#define SIGUNUSED       31

/* Real-time signals */
#define SIGRTMIN        32
#define SIGRTMAX        NSIG

#define SIG_BLOCK       0
#define SIG_UNBLOCK     1
#define SIG_SETMASK     2

#define SA_NOCLDSTOP    0x00000001
#define SA_NOCLDWAIT    0x00000002
#define SA_SIGINFO      0x00000004
#define SA_ONSTACK      0x08000000
#define SA_RESTART      0x10000000
#define SA_NODEFER      0x40000000
#define SA_RESETHAND    0x80000000

#define SIG_DFL         ((void (*)(int))0)
#define SIG_IGN         ((void (*)(int))1)
#define SIG_ERR         ((void (*)(int))-1)

typedef struct {
    uint32_t sig[_NSIG_WORDS];
} sigset_t;

struct sigaction {
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, void *info, void *context);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

struct sigpending {
    sigset_t signal;
    // TODO: List of sigqueue structures
};

struct k_sigaction {
    struct sigaction sa;
};

static struct k_sigaction sig_actions[_NSIG];
static sigset_t blocked_sigs;

void sigemptyset(sigset_t *set) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        set->sig[i] = 0;
    }
}

void sigfillset(sigset_t *set) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        set->sig[i] = ~0U;
    }
}

int sigaddset(sigset_t *set, int signum) {
    if (signum < 1 || signum > NSIG) return -1;
    set->sig[(signum - 1) / _NSIG_BPW] |= (1U << ((signum - 1) % _NSIG_BPW));
    return 0;
}

int sigdelset(sigset_t *set, int signum) {
    if (signum < 1 || signum > NSIG) return -1;
    set->sig[(signum - 1) / _NSIG_BPW] &= ~(1U << ((signum - 1) % _NSIG_BPW));
    return 0;
}

int sigismember(const sigset_t *set, int signum) {
    if (signum < 1 || signum > NSIG) return 0;
    return (set->sig[(signum - 1) / _NSIG_BPW] & (1U << ((signum - 1) % _NSIG_BPW))) != 0;
}

void sigorset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        dest->sig[i] = left->sig[i] | right->sig[i];
    }
}

void sigandset(sigset_t *dest, const sigset_t *left, const sigset_t *right) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        dest->sig[i] = left->sig[i] & right->sig[i];
    }
}

void signotset(sigset_t *dest, const sigset_t *src) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        dest->sig[i] = ~src->sig[i];
    }
}

bool sigisempty(const sigset_t *set) {
    for (int i = 0; i < _NSIG_WORDS; i++) {
        if (set->sig[i] != 0) return false;
    }
    return true;
}

void signal_init(void) {
    sigemptyset(&blocked_sigs);
    
    for (int i = 0; i < _NSIG; i++) {
        sig_actions[i].sa.sa_handler = SIG_DFL;
        sigemptyset(&sig_actions[i].sa.sa_mask);
        sig_actions[i].sa.sa_flags = 0;
    }
    
    // SIGKILL and SIGSTOP cannot be caught, blocked, or ignored
    // These are handled specially
}

int do_sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
    if (sig < 1 || sig > NSIG) return -1; // EINVAL
    if (sig == SIGKILL || sig == SIGSTOP) return -1; // EINVAL
    
    if (oact) {
        *oact = sig_actions[sig].sa;
    }
    
    if (act) {
        sig_actions[sig].sa = *act;
        // TODO: Handle SA_SIGINFO
        // TODO: Handle SA_RESETHAND
    }
    
    return 0;
}

long sys_sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
    return do_sigaction(sig, act, oact);
}

long sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (oldset) {
        *oldset = blocked_sigs;
    }
    
    if (set) {
        switch (how) {
            case SIG_BLOCK:
                sigorset(&blocked_sigs, &blocked_sigs, set);
                break;
            case SIG_UNBLOCK:
                {
                    sigset_t tmp;
                    signotset(&tmp, set);
                    sigandset(&blocked_sigs, &blocked_sigs, &tmp);
                }
                break;
            case SIG_SETMASK:
                blocked_sigs = *set;
                break;
            default:
                return -1; // EINVAL
        }
    }
    
    return 0;
}

long sys_sigpending(sigset_t *set) {
    // TODO: Return pending signals
    sigemptyset(set);
    return 0;
}

long sys_sigsuspend(const sigset_t *mask) {
    // TODO: Save old mask, set new mask, pause
    return -EINTR;
}

long sys_sigaltstack(const void *ss, void *oss) {
    // TODO: Set/get signal stack
    return 0;
}

long sys_kill(int pid, int sig) {
    // TODO: Send signal to process
    // TODO: Permission checks
    // TODO: Handle pid == 0, pid == -1, pid < -1
    
    struct task_struct *p = find_task_by_pid(pid);
    if (!p) return -ESRCH;
    
    // TODO: Deliver signal
    // TODO: Wake up process if blocked
    
    return 0;
}

long sys_tkill(int pid, int sig) {
    // TODO: Send signal to specific thread
    return sys_kill(pid, sig);
}

long sys_tgkill(int tgid, int pid, int sig) {
    // TODO: Send signal to thread group
    // TODO: Check tgid matches
    return sys_kill(pid, sig);
}

void send_sig(int sig, struct task_struct *p, int priv) {
    // TODO: Send signal to process
    // TODO: Handle priv (from kernel or user)
}

void force_sig(int sig) {
    // TODO: Force signal delivery (cannot be blocked/ignored)
}

void force_sigsegv(int sig) {
    // TODO: Force SIGSEGV
    force_sig(SIGSEGV);
}

void force_fatal_sig(int sig) {
    // TODO: Force fatal signal
    force_sig(sig);
}

bool fatal_signal_pending(struct task_struct *p) {
    // TODO: Check if fatal signal is pending
    return false;
}

bool signal_pending(struct task_struct *p) {
    // TODO: Check if any signal is pending
    return false;
}

void clear_thread_flag(int flag) {
    // TODO: Clear thread flag
}

void recalc_sigpending(void) {
    // TODO: Recalculate whether signals are pending
}

void flush_signals(struct task_struct *t) {
    // TODO: Clear all pending signals
}

void ignore_signals(struct task_struct *t) {
    // TODO: Set all signals to SIG_IGN
}

void flush_signal_handlers(struct task_struct *t, int force_default) {
    // TODO: Reset signal handlers
}

void block_all_signals(void) {
    sigfillset(&blocked_sigs);
}

void unblock_all_signals(void) {
    sigemptyset(&blocked_sigs);
}

int kill_pid_info(int sig, void *info, struct task_struct *pid) {
    // TODO: Send signal with info
    return 0;
}

int kill_pid(struct task_struct *pid, int sig, int priv) {
    return kill_pid_info(sig, NULL, pid);
}

void __group_send_sig_info(int sig, void *info, struct task_struct *p) {
    // TODO: Send signal to thread group
}

int do_send_sig_info(int sig, void *info, struct task_struct *p, bool group) {
    // TODO: Send signal with full info
    return 0;
}

void signal_wake_up(struct task_struct *t, bool resume) {
    // TODO: Wake up process for signal delivery
    t->state = TASK_RUNNING;
    // TODO: Add to runqueue
}

void ptrace_signal_deliver(struct task_struct *t, void *regs, int sig) {
    // TODO: Deliver signal to ptraced process
}

void do_signal(struct task_struct *tsk) {
    // TODO: Check and deliver pending signals
    // TODO: Handle signal frame setup
    // TODO: Handle restart_syscall
}

void exit_signals(struct task_struct *tsk) {
    // TODO: Flush and ignore signals on exit
}

void do_notify_resume(struct task_struct *tsk) {
    // TODO: Handle pending signals and work
    if (signal_pending(tsk)) {
        do_signal(tsk);
    }
}

void get_signal(struct task_struct *tsk) {
    // TODO: Get next signal to deliver
    // TODO: Handle stop signals
    // TODO: Handle core dumps
    // TODO: Handle group stops
}

void setup_sigcontext(void *sc, void *regs, int set) {
    // TODO: Setup signal context
}

void setup_frame(int sig, struct k_sigaction *ka, sigset_t *set, void *regs) {
    // TODO: Setup signal frame on user stack
}

void setup_rt_frame(int sig, struct k_sigaction *ka, void *info, sigset_t *set, void *regs) {
    // TODO: Setup real-time signal frame
}

void sys_restart_syscall(void) {
    // TODO: Restart interrupted syscall
}

long sys_signalfd(int ufd, const sigset_t *mask, int flags) {
    // TODO: Create signalfd
    return -1; // ENOSYS
}

long sys_signalfd4(int ufd, const sigset_t *mask, int flags) {
    // TODO: Create signalfd with flags
    return sys_signalfd(ufd, mask, flags);
}