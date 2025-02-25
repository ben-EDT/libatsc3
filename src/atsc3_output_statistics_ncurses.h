/*
 * output_statistics_ncurses.h
 *
 *  Created on: Feb 7, 2019
 *      Author: jjustman
 *
 */

#ifdef _WIN32
#define __DISABLE_NCURSES__
#else
#include <sys/ioctl.h>
#endif


#include <stdarg.h>

#ifndef __DISABLE_NCURSES__
#include <ncurses.h>                    /* ncurses.h includes stdio.h - but not easily linked for android */
#endif

#include <pthread.h>
#include <stdlib.h>
#include <signal.h>


#include "atsc3_lls.h"
#include "atsc3_lls_slt_parser.h"

#ifndef ATSC3_OUTPUT_STATISTICS_NCURSES_H_
#define ATSC3_OUTPUT_STATISTICS_NCURSES_H_


#if defined (__cplusplus)
extern "C" {
#endif

#ifndef __DISABLE_NCURSES__

void ncurses_init(void);
void* ncurses_input_run_thread(void *vargp);
extern pthread_mutex_t ncurses_writer_lock;
void ncurses_mutext_init(void);

void ncurses_writer_lock_mutex_acquire(void);
void ncurses_writer_lock_mutex_release(void);
void ncurses_writer_lock_mutex_destroy(void);
void create_or_update_window_sizes(bool should_reload_term_size);
void handle_winch(int sig);
void handle_sighup(int sig);


void* print_lls_instance_table_thread(void*);
void lls_dump_instance_table_ncurses(lls_table_t* lls_session);

#else
#define ncurses_init(...)
#define ncurses_input_run_thread(...)
#define ncurses_mutext_init(...)
#define create_or_update_window_sizes(...)
#endif

#ifdef __DISABLE_NCURSES__
#define WINDOW void
#define SCREEN void
#define wnoutrefresh(...)
#define wrefresh(...)
#define doupdate(...)
#define curscr(...)
#define curscr(...)
#define vs(...)
#define werase(...)
#define wprintw(...)

#endif

#define __BW_STATS_NCURSES true
#define __PKT_STATS_NCURSES true

#ifndef __DISABLE_NCURSES__


extern SCREEN* my_screen;
//wmove(bw_window, 0, 0 __VA_ARGS__); //vwprintf(bw_window, __VA_ARGS__);
extern WINDOW* my_window;

extern WINDOW* left_window_outline;
extern	WINDOW* pkt_global_stats_window;
extern	WINDOW* signaling_global_stats_window;

extern	WINDOW* bw_window_outline;
extern		WINDOW* bw_window_runtime;
extern		WINDOW* bw_window_lifetime;


extern WINDOW* right_window_outline;
extern 	WINDOW* pkt_flow_stats_mmt_window;

extern WINDOW* bottom_window_outline;
extern 	WINDOW* pkt_global_loss_window;

#define __BW_STATS_NOUPDATE() wnoutrefresh(bw_window_runtime);					\
									wnoutrefresh(bw_window_lifetime);

#define __BW_STATS_RUNTIME(...) wprintw(bw_window_runtime, __VA_ARGS__); \
									wprintw(bw_window_runtime,"\n");

#define __BW_STATS_LIFETIME(...)	wprintw(bw_window_lifetime, __VA_ARGS__); \
									wprintw(bw_window_lifetime,"\n");

#define __BW_CLEAR() 				werase(bw_window_runtime);				\
									werase(bw_window_lifetime); \

#define __PS_STATS_NOUPDATE()		wnoutrefresh(pkt_global_stats_window);	\
									wnoutrefresh(pkt_flow_stats_mmt_window); \
									wnoutrefresh(pkt_global_loss_window);

#define __PS_STATS_GLOBAL(...) 		wprintw(pkt_global_stats_window, __VA_ARGS__); \
									wprintw(pkt_global_stats_window,"\n");

#define __PS_STATS_FLOW(...) 		wprintw(pkt_flow_stats_mmt_window, __VA_ARGS__); \
									wprintw(pkt_flow_stats_mmt_window,"\n");

#define __PS_STATS_FLOW_W(...) 		wprintw(pkt_flow_stats_mmt_window, __VA_ARGS__);


extern WINDOW* pkt_flow_stats_mmt_log_window;

#define __PS_STATS_FLOW_LOG(...) 	wprintw(pkt_flow_stats_mmt_log_window, __VA_ARGS__); \
									wprintw(pkt_flow_stats_mmt_log_window,"\n");

#define __PS_STATS_FLOW_LOG_REFRESH() wrefresh(pkt_flow_stats_mmt_log_window);


#define __PS_STATS_HR()				whline(pkt_flow_stats_mmt_window, ACS_BOARD, 15); \
									wprintw(pkt_flow_stats_mmt_window,"\n");

#define __PS_STATS_LOSS(...) 		wprintw(pkt_global_loss_window, __VA_ARGS__); 	\
									wprintw(pkt_global_loss_window,"\n");

#define __PS_REFRESH_LOSS() 		wnoutrefresh(pkt_global_loss_window);


#define __PS_REFRESH() 				wnoutrefresh(pkt_global_stats_window);			\
									wnoutrefresh(pkt_flow_stats_mmt_window);

#define __PS_CLEAR()  				werase(pkt_global_stats_window);				\
									werase(pkt_flow_stats_mmt_window);

#define __PS_STATS_GLOBAL_LOSS(...)	wprintw(pkt_global_loss_window, __VA_ARGS__); \
									wprintw(pkt_global_loss_window,"\n");



#define __LLS_DUMP_NOUPDATE() 		wnoutrefresh(signaling_global_stats_window);

#define __LLS_DUMP_CLEAR() 			werase(signaling_global_stats_window);

#define __LLS_DUMP(...)				wprintw(signaling_global_stats_window, __VA_ARGS__); \
									wprintw(signaling_global_stats_window,"\n");

#define __LLS_REFRESH() 			wnoutrefresh(signaling_global_stats_window);



extern int global_mmt_loss_count;

#define __DOUPDATE() 				doupdate(); wrefresh(curscr);
#define __DOUPDATE_MFU() 			doupdate();


#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#define __NCURSES_ERROR(...)   printf("%s:%d:ERROR :","ncurses",__LINE__);printf(__VA_ARGS__);printf("%s%s","\r","\n")
#define __NCURSES_WARN(...)    printf("%s:%d:WARN: ","ncurses",__LINE__);printf(__VA_ARGS__);printf("%s%s","\r","\n")
#define __NCURSES_INFO(...)    printf("%s:%d: ","ncurses",__LINE__);printf(__VA_ARGS__);printf("%s%s","\r","\n")


#endif

#if defined (__cplusplus)
}
#endif


#endif /* ATSC3_OUTPUT_STATISTICS_NCURSES_H_ */
