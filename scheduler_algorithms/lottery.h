#ifndef LOTTERY_H
#define LOTTERY_H

int get_next_lottery(int current_time);
void update_process(int proc_idx, int new_tickets);
void build_initial(int current_time);
void activate_arrived(int current_time);
void lottery_process_finished(int proc_idx);

#endif