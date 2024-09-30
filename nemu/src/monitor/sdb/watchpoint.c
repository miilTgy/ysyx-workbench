/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  bool used;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].used = false;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (free_->next == NULL) {
    printf("WatchpointERROR: no more space for creating watchpoints!\n");
    return NULL;
  }
  
  WP* new = free_;
  free_ = free_->next; // remove first free wp
  if (head == NULL) {
    head = new;
    head->used = true;
  } else {
    WP* temp = head;
    while (temp->next != NULL && temp->next->used != false) {
      temp = temp->next; // find the tail of head
    }
    temp->next = new; // head.append(free_); free_[0].pop();
    new->used = true;
  }
  return new;
}
void create_awatchpoint() {
  WP* newp = new_wp();
  if (newp != NULL) {
    if (newp->next != NULL) {
      printf("Create watchpoint NO. %d next %d\n", newp->NO, newp->next->NO);
    } else {
      printf("Create watchpoint NO. %d next NULL\n", newp->NO);
    }
  }
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    assert(0);
  } else if (wp == head) {
    head = head->next;
  } else {
    WP* temp = head;
    while (temp != NULL && temp->next != wp) {
      temp = temp->next; // search wp in head
    }
    temp->next = temp->next->next; // break chain from here
  }
  wp->next = free_;
  free_ = wp; // inseart wp at the begining of free_
}
/*
int delete_watchpoint(int NO) { 
  WP* temp = head;
  while (temp->used != false) {
    if (temp->NO == NO) {
      free_wp(temp);
    }
    temp = temp->next;
  }
}
*/
void desplay_wp() {
  for (size_t i = 0; i < NR_WP; i++) {
    printf("NO.%d, used: %d\n", wp_pool[i].NO, wp_pool[i].used);
  }
  
  printf("Actived watchpoints:\n");
  WP* wp = head;
  if (wp == NULL) {
    printf("\033[0;35mNo watchpoint activated!\033[m\n");
  } else {
    while (wp->used != false && wp->next != NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, next=\033[0;35m%d\033[m\n", wp->NO, wp->next->NO);
      wp = wp->next;
    }
    if (wp->next == NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, next=\033[0;35mNULL\033[m\n", wp->NO);
    }
  }
  printf("Freed watchpoints:\n");
  WP *nowp = free_;
  if (nowp == NULL || nowp->used == true) {
    printf("\033[0;35mNo watchpoint freed!\033[m\n");
  } else {
    while (nowp->used == false && nowp->next != NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, next=\033[0;35m%d\033[m\n", nowp->NO, nowp->next->NO);
      nowp = nowp->next;
    }
    if (nowp->next == NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, next=\033[0;35mNULL\033[m\n", nowp->NO);
    }
  }
  
}