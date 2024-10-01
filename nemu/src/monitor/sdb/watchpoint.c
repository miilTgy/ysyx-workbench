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
  struct watchpoint *head_next;
  struct watchpoint *free_next;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].free_next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].used = false;
    wp_pool[i].head_next = NULL;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (free_->free_next == NULL) {
    printf("WatchpointERROR: no more space for creating watchpoints!\n");
    return NULL;
  }
  
  WP* new = free_;
  free_ = free_->free_next; // remove first free wp
  if (head == NULL) {
    head = new;
    head->used = true;
  } else {
    WP* temp = head;
    while (temp->head_next != NULL) {
      temp = temp->head_next; // find the tail of head
    }
    printf("end of head is %d\n", temp->NO);
    temp->head_next = new; // head.append(free_); free_[0].pop();
    new->head_next = NULL; // set new as end of head
    new->used = true;
  }
  new->free_next = NULL;
  return new;
}
void create_awatchpoint() {
  WP* newp = new_wp();
  if (newp != NULL) {
    if (newp->head_next != NULL) {
      printf("Create watchpoint NO. %d next %d\n", newp->NO, newp->head_next->NO);
    } else {
      printf("Create watchpoint NO. %d next NULL\n", newp->NO);
    }
  }
}

void free_wp(WP *wp) {
  if (wp->used == false) {
    printf("Watchpoint unused\n");
    return;
  } else if (wp == head) {
    if (head->head_next == NULL) {
      head = NULL;
    } else {
      head = head->head_next;
    }
    wp->used = false;
  } else {
    WP* temp = head;
    while (temp != NULL && temp->head_next != wp) {
      temp = temp->head_next; // search wp in head
    }
    temp->head_next = temp->head_next->head_next; // break chain from here
  }
  wp->used = false;
  wp->head_next = NULL;
  wp->free_next = free_;
  free_ = wp; // inseart wp at the begining of free_
}

int delete_watchpoint(int NO) { 
  for (size_t i = 0; i < NR_WP; i++) {
    if (wp_pool[i].NO == NO && wp_pool[i].used) {
      free_wp(&wp_pool[i]);
      printf("Delete watchpoint NO.%d\n", NO);
      return 0;
    }
  }
  printf("Watchpoint NO.%d not activated!\n", NO);
  return 0;
}

void desplay_wp() {
  if (head == NULL) {
    printf("head=NULL\n");
  } else {
    printf("head=%d\n", head->NO);
  }
  if (free_ == NULL) {
    printf("free_=NULL\n");
  } else {
    printf("free_=%d\n", free_->NO);
  }
 
  
  for (size_t i = 0; i < NR_WP; i++) {
    printf("NO.%d, used: %d, ", wp_pool[i].NO, wp_pool[i].used);
    if (wp_pool[i].next == NULL) {
      printf("next: NULL, ");
    } else {
      printf("next: %d, ", wp_pool[i].next->NO);
    }
    if (wp_pool[i].head_next == NULL) {
      printf("hnext: NULL, ");
    } else {
      printf("hnext: %d, ", wp_pool[i].head_next->NO);
    }
    if (wp_pool[i].free_next == NULL) {
      printf("fnext: NULL\n");
    } else {
      printf("fnext: %d\n", wp_pool[i].free_next->NO);
    }
  }
  
  printf("Actived watchpoints:\n");
  WP* wp = head;
  if (wp == NULL) {
    printf("\033[0;35mNo watchpoint activated!\033[m\n");
  } else {
    while (wp->used != false && wp->head_next != NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, hnext=\033[0;35m%d\033[m\n", wp->NO, wp->head_next->NO);
      wp = wp->head_next;
    }
    if (wp->head_next == NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, hnext=\033[0;35mNULL\033[m\n", wp->NO);
    }
  }
  printf("Freed watchpoints:\n");
  WP *nowp = free_;
  if (nowp == NULL || nowp->used == true) {
    printf("\033[0;35mNo watchpoint freed!\033[m\n");
  } else {
    while (nowp->used == false && nowp->free_next != NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, fnext=\033[0;35m%d\033[m\n", nowp->NO, nowp->free_next->NO);
      nowp = nowp->free_next;
    }
    if (nowp->free_next == NULL) {
      printf("  Watchpoint No.\033[0;35m%d\033[m, fnext=\033[0;35mNULL\033[m\n", nowp->NO);
    }
  }
  
}