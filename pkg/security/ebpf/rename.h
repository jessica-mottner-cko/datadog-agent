#ifndef _RENAME_H_
#define _RENAME_H_

#include "syscalls.h"

struct rename_event_t {
    struct event_t event;
    struct process_data_t process;
    dev_t         dev;
    u32           padding;
    unsigned long src_inode;
    unsigned long target_inode;
};

int __attribute__((always_inline)) trace__sys_rename() {
    struct syscall_cache_t syscall = {};
    cache_syscall(&syscall);

    return 0;
}

SYSCALL_KPROBE(rename) {
    return trace__sys_rename();
}

SYSCALL_KPROBE(renameat) {
    return trace__sys_rename();
}

SYSCALL_KPROBE(renameat2) {
    return trace__sys_rename();
}

SEC("kprobe/vfs_rename")
int kprobe__vfs_rename(struct pt_regs *ctx) {
    struct syscall_cache_t *syscall = peek_syscall();
    if (!syscall)
        return 0;

    syscall->rename.src_dir = (struct inode *)PT_REGS_PARM1(ctx);
    syscall->rename.src_dentry = (struct dentry *)PT_REGS_PARM2(ctx);

    // we generate a fake source key as the inode is (can be ?) reused
    syscall->rename.random_key.dev = 0xffffffff;
    syscall->rename.random_key.ino = bpf_get_prandom_u32() << 32 | bpf_get_prandom_u32();
    resolve_dentry(syscall->rename.src_dentry, syscall->rename.random_key);

    return 0;
}

int __attribute__((always_inline)) trace__sys_rename_ret(struct pt_regs *ctx) {
    struct syscall_cache_t *syscall = pop_syscall();
    if (!syscall)
        return 0;

    int retval = PT_REGS_RC(ctx);
    if (IS_UNHANDLED_ERROR(retval))
        return 0;

    struct path_key_t path_key = get_dentry_key(syscall->rename.src_dentry);
    struct rename_event_t event = {
        .event.retval = retval,
        .event.type = EVENT_VFS_RENAME,
        .event.timestamp = bpf_ktime_get_ns(),
        .dev = path_key.dev,
        .src_inode = syscall->rename.random_key.ino,
        .target_inode = path_key.ino,
    };

    fill_process_data(&event.process);
    resolve_dentry(syscall->rename.src_dentry, path_key);

    send_event(ctx, event);

    return 0;
}

SYSCALL_KRETPROBE(rename) {
    return trace__sys_rename_ret(ctx);
}

SYSCALL_KRETPROBE(renameat) {
    return trace__sys_rename_ret(ctx);
}

SYSCALL_KRETPROBE(renameat2) {
    return trace__sys_rename_ret(ctx);
}

#endif