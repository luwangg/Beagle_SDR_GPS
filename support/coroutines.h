//////////////////////////////////////////////////////////////////////////
// Homemade GPS Receiver
// Copyright (C) 2013 Andrew Holme
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// http://www.holmea.demon.co.uk/GPS/Main.htm
//////////////////////////////////////////////////////////////////////////

#ifndef	_COROUTINES_H_
#define	_COROUTINES_H_

#include "types.h"
#include "config.h"
#include "timing.h"

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define	HIGHEST_PRIORITY	7

#define	SPIPUMP_PRIORITY	7
#define	DATAPUMP_PRIORITY	6
#define	SND_PRIORITY		5
#define	WF_OVERLAP_PRIORITY	4
#define	WF_PRIORITY			2
#define ADMIN_PRIORITY		2
#define	WEBSERVER_PRIORITY	2
#define	APPS_PRIORITY		2
#define	GPS_PRIORITY		2
#define GPS_ACQ_PRIORITY	2
#define	MAIN_PRIORITY		2

#define	LOWEST_PRIORITY		0
#define	NUM_PRIORITY		(HIGHEST_PRIORITY+1)

union regs_t {
	jmp_buf jb;
	struct {
		#if defined(__x86_64__)
			u_int64_t x1, fp, sp, x2[4], pc;
		#endif
		#if defined(__ARM_EABI__)
			u_int32_t v[6], sl, fp, sp, pc;
		#endif
	};
};

typedef void (*func_t)();
typedef void (*funcP_t)(void *);

void TaskInit();

#define CTF_BUSY_HELPER		0x0001
#define CTF_POLL_INTR		0x0002

#define CreateTask(f, priority)				_CreateTask(f, #f, priority, 0)
#define CreateTaskSP(f, s, priority, param)	_CreateTaskP(f, s, priority, param)
#define CreateTaskF(f, priority, flags)		_CreateTask(f, #f, priority, flags)
#define CreateTaskP(f, priority, param)		_CreateTaskP(f, #f, priority, param)
int _CreateTask(func_t entry, const char *name, int priority, u4_t flags);
int _CreateTaskP(funcP_t entry, const char *name, int priority, void *param);

int TaskSleep(int usec);
void TaskSleepID(int id, int usec);
void TaskWakeup(int id, bool check_waking, int wake_param);

enum ipoll_from_e {
	CALLED_WITHIN_NEXTTASK, CALLED_AFTER_HOLDOFF, CALLED_FROM_LOCK, CALLED_FROM_SPI
};
void TaskPollForInterrupt(ipoll_from_e from);
void TaskInterruptHoldoff(bool holdoff);
extern u64_t interrupt_task_last_run;

void TaskRemove(int id);
void TaskParams(u4_t quanta_us);
void TaskDump();
void TaskLastRun();
u4_t TaskID();
u4_t TaskPriority(int priority);
const char *TaskName();
void TaskCheckStacks();

#define	TSTAT_MASK		0x00ff
#define	TSTAT_NC		0
#define	TSTAT_SET		1
#define	TSTAT_INCR		2
#define	TSTAT_MAX		3

#define	TSTAT_LATCH		0x0f00
#define	TSTAT_ZERO		0x0100
#define	TSTAT_CMDS		0x0200

#define TSTAT_SPI_RETRY	0x1000

int TaskStatU(u4_t s1_func, int s1_val, const char *s1_units, u4_t s2_func, int s2_val, const char *s2_units);
#define TaskStat(f1, v1, f2, v2) TaskStatU(f1, v1, NULL, f2, v2, NULL)

#define	NT_NONE			0
#define	NT_BUSY_WAIT	1
#define	NT_LONG_RUN		2

#ifdef DEBUG
 void _NextTask(const char *s, u4_t param, u_int64_t pc);
#else
 void _NextTask(u4_t param);
#endif

#ifdef DEBUG
 //#define NextTask(s)		_NextTask(s, NT_NONE, 0);
 #define NextTask(s)		NextTaskW(s, NT_NONE);

 //#define NextTaskP(s,p)	_NextTask(s, p, 0);
 #define NextTaskP(s,p)		NextTaskW(s, p);

 #define NextTaskW(s,p)	{ \
 	regs_t regs; \
 	setjmp(regs.jb); /* get pc of caller to NextTask() */ \
 	_NextTask(s, p, regs.pc); \
 	}
#else
 #define NextTask(s)		_NextTask(NT_NONE);
 #define NextTaskP(s,p)		_NextTask(p);
 #define NextTaskW(s,p)		_NextTask(p);
#endif

#define LOCK_MAGIC_B	0x10ccbbbb
#define LOCK_MAGIC_E	0x10cceeee

typedef struct {
	u4_t magic_b;
	bool init;
	u4_t enter, leave;
	const char *name;
	char enter_name[64];
	int tid;
	const char *tname;
	u4_t magic_e;
} lock_t;

#define lock_init(lock) _lock_init(lock, #lock)
void _lock_init(lock_t *lock, const char *name);
void lock_enter(lock_t *lock);
void lock_leave(lock_t *lock);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif