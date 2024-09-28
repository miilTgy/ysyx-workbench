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
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  WP* new = free_;
  free_ = free_->next; // remove first free wp
  if (head == NULL) {
    head = new;
  } else {
    WP* temp = head;
    while (temp->next != NULL) {
      temp = temp->next; // find the tail of head
    }
    temp->next = new; // head.append(free_); free_[0].pop();
  }
  return new;
}
void create_awatchpoint() {
  WP* newp = new_wp();
  printf("Successfully create watchpoint NO. %d next %d\n", newp->NO, newp->next->NO);
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
