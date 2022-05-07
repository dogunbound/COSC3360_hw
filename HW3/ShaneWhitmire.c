#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER_SIZE 256
#define SET_STRATEGY_FIFO 'a'
#define SET_STRATEGY_LRU 'b'
#define SET_STRATEGY_LFU 'c'

typedef struct String {
  char *_buf;
  size_t _len, _max_size;
  void (*clear)(struct String *self);
  void (*set)(struct String *self, char* new_str);
  void (*print)(struct String *self);
  void (*trim)(struct String *self);
  void (*free)(struct String *self);
  bool (*is_empty)(struct String *self);
  bool (*equal)(struct String *self, struct String *other);
  struct String (*copy)(struct String *self);
} String;
String malloc_string(unsigned int size);

bool check_string_equal(struct String *a, struct String *b) {
  if (a->_len != b->_len)
    return false;

  for (size_t i = 0; i < a->_len; i++) {
    if (a->_buf[i] != b->_buf[i])
      return false;
  }
  return true;
}

bool is_empty_string(String *self) {
  return self->_len == 0;
}

void free_string(String *self) {
  if (self->_buf)
    free(self->_buf);
}

bool trim_string_helper_char_is_trailing(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\0');
}

void trim_string(String *self) {
  size_t shift = 0;
  bool first_set_of_spaces_cleared = false;
  for (size_t i = 0; i < self->_len; i++) {
    if (!first_set_of_spaces_cleared && trim_string_helper_char_is_trailing(self->_buf[i])) {
      shift++;
    } else {
      first_set_of_spaces_cleared = true;
      self->_buf[i-shift] = self->_buf[i];
    }
    if (strcmp(&self->_buf[i], "\0") == 0) {
      break;
    }
  }

  for (size_t i = self->_len;
      i >= 0 && trim_string_helper_char_is_trailing(self->_buf[i]);
      i--) {
    self->_len--;
  }
  self->_len++;

  for (size_t i = self->_len; i < self->_max_size; i++) {
    self->_buf[i] = '\0';
  }
}

void print_string(String *self) {
  printf("%s", self->_buf);
}

void set_string(String *self, char* new_str) {
  self->clear(self);
  self->_len = strlen(new_str);
  self->_len = (self->_len > self->_max_size) ? self->_max_size : self->_len;
  for (size_t i = 0; i < self->_len; i++) {
    self->_buf[i] = new_str[i];
  }
}


void clear_string(String *self) {
  for (size_t i = 0; i < self->_max_size; i++) {
    self->_buf[i] = '\0';
  }
  self->_len = 0;
}

String copy_string(String *self) {
  struct String cpy = malloc_string(self->_max_size);
  cpy._len = self->_len;
  for (size_t i = 0; i < self->_len; i++) {
    cpy._buf[i] = self->_buf[i];
  }

  return cpy;
}

String malloc_string(unsigned int size) {
  char *_buf = (char*)malloc(sizeof(char) * size);
  String string = {
    ._buf = _buf,
    ._max_size = size,
    ._len = 0,
    .clear = clear_string,
    .set = set_string,
    .print = print_string,
    .trim = trim_string,
    .free = free_string,
    .is_empty = is_empty_string,
    .equal = check_string_equal,
    .copy = copy_string,
  };

  string.clear(&string);

  return string;
}

struct PidToPageFrames {
  int pid;
  struct String memory_addr;
  void (*print)(struct PidToPageFrames *self);
  void (*free)(struct PidToPageFrames *self);
  bool (*is_hex)(struct PidToPageFrames *self);
  bool (*equal)(struct PidToPageFrames *self, struct PidToPageFrames *other);
  struct PidToPageFrames (*copy)(struct PidToPageFrames *self);
};

struct PidToPageFrames copy_page_frames(struct PidToPageFrames *self) {
  struct String memory_addr = self->memory_addr.copy(&self->memory_addr);

  struct PidToPageFrames cpy = {
    .pid = self->pid,
    .memory_addr = memory_addr,
  };

  return cpy;
}

struct PidToPageFrames construct_pid_to_page_frames(); 

bool check_pid_to_page_frames_is_equal(struct PidToPageFrames *a, struct PidToPageFrames *b) {
  if (a->pid != b->pid) return false;
  if (!a->memory_addr.equal(&a->memory_addr, &b->memory_addr)) return false;
  return true;
}

bool is_hex_pid_to_page_frames(struct PidToPageFrames *self) {
  for (size_t i = 0; i < self->memory_addr._len; i++)
    if (self->memory_addr._buf[i] == 'x' || self->memory_addr._buf[i] == 'X')
      return true;

  return false;
}

void free_pid_to_page_frames(struct PidToPageFrames *self) {
  self->memory_addr.free(&self->memory_addr);
}

void print_pid_to_page_frames(struct PidToPageFrames *self) {
  printf("Process id: %d\n", self->pid);
  printf("Frames: ");
  self->memory_addr.print(&self->memory_addr);
  printf("\nIs hex: %s", self->is_hex(self) ? "true" : "false" );
  printf("\n");
}

struct PidToPageFrames construct_pid_to_page_frames() {
  struct PidToPageFrames tmp = {
    .print = print_pid_to_page_frames,
    .free = free_pid_to_page_frames,
    .is_hex = is_hex_pid_to_page_frames,
    .equal = check_pid_to_page_frames_is_equal,
  };

  return tmp;
}

struct PidToPageFrames malloc_pid_to_page_frames(size_t frame_id_buf_size) {
  struct PidToPageFrames tmp = construct_pid_to_page_frames();
  tmp.memory_addr = malloc_string(frame_id_buf_size);
  return tmp;
}

struct MainState {
  int total_number_of_page_frames;
  int page_size;
  int number_of_page_frames_per_process;
  int lookahead_window_size;
  int min_pool_size;
  int max_pool_size;
  int *process_nums;
  int num_of_processes;
  struct PidToPageFrames *working_set;
  int size_of_working_set;
  void (*print)(struct MainState *self);
  struct MainState (*copy)(struct MainState *self);
};

struct MainState construct_main_state();

// This function doesn't work, and I gave up trying to use it
struct MainState copy_main_state(struct MainState *self) {
  struct MainState cpy = construct_main_state();
  cpy.total_number_of_page_frames = self->total_number_of_page_frames;
  cpy.page_size = self->page_size;
  cpy.number_of_page_frames_per_process = self->number_of_page_frames_per_process;
  cpy.lookahead_window_size = self->lookahead_window_size;
  cpy.min_pool_size = self->min_pool_size;
  cpy.max_pool_size = self->max_pool_size;
  cpy.num_of_processes = self->num_of_processes;
  cpy.size_of_working_set = self->size_of_working_set;

  cpy.working_set = malloc(sizeof(struct MainState) * cpy.size_of_working_set);
  for (size_t i = 0; i < cpy.size_of_working_set; i++) {
    cpy.working_set[i] = construct_pid_to_page_frames();
    cpy.working_set[i].pid = self->working_set[i].pid;
    cpy.working_set[i].memory_addr = self->working_set[i].memory_addr.copy(&self->working_set[i].memory_addr);
  }

  return cpy;
}

void print_main_state(struct MainState *self) {
  printf("\n==========================================================\n");
  printf("Total number of page frames: %d\n", self->total_number_of_page_frames);
  printf("Page size: %d\n", self->page_size);
  printf("Number of page frames per process: %d\n", self->number_of_page_frames_per_process);
  printf("Look ahead window size: %d\n", self->lookahead_window_size);
  printf("Minimum pool size: %d\n", self->min_pool_size);
  printf("Maximum pool size: %d\n", self->max_pool_size);
  printf("Number of processes: %d\n", self->num_of_processes);
  printf("Process nums: [ %d", self->process_nums[0]);
  for (size_t i = 1; i < self->num_of_processes; i++) {
    printf(", %d", self->process_nums[i]);
  }
  printf("]\n");

  printf("\nWorkingState:\n");
  for (size_t i = 0; i < self->size_of_working_set; i++) {
    self->working_set[i].print(&self->working_set[i]);
    printf("-----------\n");
  }
  printf("==========================================================\n");
}

struct MainState construct_main_state() {
  struct MainState main_state = {
    .print = print_main_state,
    .copy = copy_main_state,
  };

  return main_state;
}

struct MainState process_input(char *dir) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  size_t read;

  struct MainState main_state = construct_main_state();

  main_state.size_of_working_set = 0;

  fp = fopen(dir, "r");

  if (fp == NULL)
    exit(EXIT_FAILURE);

  size_t num_of_ppfs = -7;
  while(!feof(fp)) {
    char ch = fgetc(fp);
    if(ch == '\n')
      num_of_ppfs++;
  }

  struct String *instructions = malloc(sizeof(String) * num_of_ppfs);
  rewind(fp);
  size_t index = 0;
  while((read = getline(&line, &len, fp)) != -1) {
    switch (index) {
      case 0:
        main_state.total_number_of_page_frames = atoi(line);
        break;
      case 1:
        main_state.page_size = atoi(line);
        break;
      case 2:
        main_state.number_of_page_frames_per_process = atoi(line);
        break;
      case 3:
        main_state.lookahead_window_size = atoi(line);
        break;
      case 4:
        main_state.min_pool_size = atoi(line);
        break;
      case 5:
        main_state.max_pool_size = atoi(line);
        break;
      case 6:
        main_state.num_of_processes = atoi(line);
        break;
      default:
        instructions[index - 7] = malloc_string(64);
        instructions[index - 7].set(&instructions[index - 7], line);
        break;
    }
    index++;
  }
  fclose(fp);
  if(line)
    free(line);

  int *process_nums = malloc(sizeof(int) * num_of_ppfs);
  for (size_t i = 0; i < num_of_ppfs; i++) process_nums[i] = -1;

  struct PidToPageFrames *ppfs = malloc(sizeof(struct PidToPageFrames) * num_of_ppfs);
  for (size_t i = 0; i < num_of_ppfs; i++) {
    instructions[i].trim(&instructions[i]);
    size_t first_space = 0;
    for (size_t j = 0; j < instructions[i]._len; j++) {
      if (instructions[i]._buf[j] == ' ') {
        first_space = j;
        break;
      }
    }

    char pid[16];
    memcpy(pid, &instructions[i]._buf[0], first_space);
    char memory_addr[BUFFER_SIZE];
    memcpy(memory_addr, &instructions[i]._buf[first_space+1], instructions[i]._len);

    struct String frame_string = malloc_string(BUFFER_SIZE);
    frame_string.set(&frame_string, memory_addr);
    frame_string.trim(&frame_string);

    int int_pid = atoi(pid);
    ppfs[i] = construct_pid_to_page_frames();
    ppfs[i].pid = int_pid;
    ppfs[i].memory_addr = frame_string;
    instructions[i].free(&instructions[i]);

    // process_nums
    for (size_t j = 0; j < num_of_ppfs; j++) {
      if (process_nums[j] == -1) {
        process_nums[j] = int_pid;
        break;
      } else if (process_nums[j] == int_pid) {
        break;
      }
    }
    printf("\n");
  }

  size_t count = 0;
  for (size_t i = 0; i < num_of_ppfs; i++) {
    if (process_nums[i] == -1) break;
    count++;
  }

  process_nums = (int *) realloc(process_nums, sizeof(int) * count);

  main_state.process_nums = process_nums;
  main_state.working_set = ppfs;
  main_state.size_of_working_set = num_of_ppfs;
  main_state.copy = copy_main_state;

  return main_state;
}

struct ReplacementValueNodes {
  int id;
  size_t lfu_count;
  struct ReplacementValueNodes *next;
  struct ReplacementValueNodes *prev;
};

typedef struct Paging {
  struct PidToPageFrames *pages;
  struct ReplacementValueNodes *front_replacement_values;
  struct ReplacementValueNodes *back_replacement_values;
  int FIFO_counter;
  int *ids_used;
  int lookahead_window_size;
  int min_pool_size;
  int max_pool_size;
  char replacement_strategy;
  size_t num_of_pages;
  bool (*request)(struct Paging *self, struct PidToPageFrames *page); // True if fault occurs. False is no fault occurs
  void (*print)(struct Paging *self);
  void (*free)(struct Paging *self);
  void (*move_back)(struct Paging *self, struct ReplacementValueNodes *tmp);
} Paging;

void move_back_paging_replacement_node(struct Paging *self, struct ReplacementValueNodes *tmp) {
  if (tmp == self->back_replacement_values) return; // if tmp is already at back, do nothing
  if (tmp == self->front_replacement_values) {
    self->front_replacement_values = tmp->next;
    self->front_replacement_values->prev = NULL;

    tmp->prev = self->back_replacement_values;
    self->back_replacement_values->next = tmp;
    
    tmp->next = NULL;
    self->back_replacement_values = tmp;
    return;
  }

  struct ReplacementValueNodes *tmp_next = tmp->next;
  struct ReplacementValueNodes *tmp_prev = tmp->prev;
  if (tmp_next != NULL)
    tmp_next->prev = tmp_prev;
  if (tmp_prev != NULL)
    tmp_prev->next = tmp_next;

  tmp->prev = self->back_replacement_values;
  tmp->next = NULL;
  self->back_replacement_values->next = tmp;
  self->back_replacement_values = tmp;
}

void print_frames(struct Paging *self) {
  printf("*******************************************************************\n");
  printf("Paging info:\n");
  printf("Number of pages: %zu\n", self->num_of_pages);
  printf("look ahead window size: %d\n", self->lookahead_window_size);
  printf("Minimum pool size: %d\n", self->min_pool_size);
  printf("Maximum pool size: %d\n", self->max_pool_size);
  printf("Frames: [ {%d, %s}", self->pages[0].pid, self->pages[0].memory_addr._buf);
  for (size_t i = 1; i < self->num_of_pages; i++) {
    printf(", {%d, %s}", self->pages[i].pid, self->pages[i].memory_addr._buf);
  }
  printf(" ]\n");

  struct ReplacementValueNodes *tmp;
  switch (self->replacement_strategy) {
    case SET_STRATEGY_FIFO:
      printf("Replacement value: %d\n", self->FIFO_counter);
      break;
    case SET_STRATEGY_LRU:
      printf("Replacement values: [ %d", self->front_replacement_values->id);
      tmp = self->front_replacement_values->next;
      while (tmp != NULL) {
        printf(", %d", tmp->id);
        tmp = tmp->next;
      }
      printf(" ]\n");
      break;
    case SET_STRATEGY_LFU:
      printf("Replacement values: [ {%d, %zu}", self->front_replacement_values->id, self->front_replacement_values->lfu_count);
      tmp = self->front_replacement_values->next;
      while (tmp != NULL) {
        printf(", {%d %zu}", tmp->id, tmp->lfu_count);
        tmp = tmp->next;
      }
      printf(" ]\n");
      break;
  }
  printf("*******************************************************************\n");
}

void free_frames(struct Paging *self) {
  for (size_t i = 0; i < self->num_of_pages; i++) {
    self->pages[i].free(&self->pages[i]);
  }
  free(self->pages);
  free(self->ids_used);
  while (self->front_replacement_values->next != NULL) {
    struct ReplacementValueNodes *tmp = self->front_replacement_values;
    self->front_replacement_values = self->front_replacement_values->next;
    free(tmp);
  }
}

// True if fault occurs. False if no fault occurs
bool LFU(struct Paging *self, struct PidToPageFrames *page) {
  bool hit = false;

  size_t page_id = -1;
  struct ReplacementValueNodes *tmp = self->back_replacement_values;
  while (tmp != NULL) {
    page_id++;
    if (tmp->id == -1) { // If id = -1, this will ALWAYS be a page fault. This means we are at the end of allocated memory. All memory after this is unallocated.
      self->pages[page_id].pid = page->pid;
      self->pages[page_id].memory_addr = page->memory_addr.copy(&page->memory_addr);
      tmp->id = page_id;
      tmp->lfu_count = 1;
      self->move_back(self, tmp);
      return !hit;
    } else if (self->pages[tmp->id].equal(&self->pages[tmp->id], page)) {
      hit = true;
      tmp->lfu_count++;
      if (tmp == self->back_replacement_values) return !hit;

      struct ReplacementValueNodes *searcher = tmp;
      while (searcher != self->back_replacement_values && searcher->lfu_count > searcher->next->lfu_count) {
        searcher = searcher->next;
      }

      if (searcher == tmp) return !hit; // tmp is already at arrival point. No need to change order! This also takes care of the edge case of being in front
      struct ReplacementValueNodes *tmp_next = tmp->next;
      struct ReplacementValueNodes *tmp_prev = tmp->prev;

      if (searcher == self->back_replacement_values) {
        self->back_replacement_values = tmp;
        tmp_next->prev = tmp_prev;
        tmp_prev->next = tmp_next;

        tmp->prev = searcher;
        searcher->next = tmp;

        tmp->next = NULL;
        self->back_replacement_values = tmp;
      } else {
        tmp_next->prev = tmp_prev;
        tmp_prev->next = tmp_next;

        searcher->next->prev = tmp;
        tmp->next = searcher->next;
        searcher->next = tmp;
        tmp->prev = searcher;
      }

      return !hit;
    }

    tmp = tmp->prev;
  }

  self->pages[self->front_replacement_values->id].pid = page->pid;
  self->pages[self->front_replacement_values->id].free(&self->pages[self->front_replacement_values->id]);
  self->pages[self->front_replacement_values->id].memory_addr = page->memory_addr.copy(&page->memory_addr);
  self->front_replacement_values->lfu_count = 1;

  return !hit;
}

// True if fault occurs. False if no fault occurs
bool LRU(struct Paging *self, struct PidToPageFrames *page) {
  bool hit = false;

  size_t page_id = -1;
  struct ReplacementValueNodes *tmp = self->back_replacement_values;
  while (tmp != NULL) {
    page_id++;
    if (tmp->id == -1) { // If id = -1, this will ALWAYS be a page fault. This means we are at the end of allocated memory. All memory after this is unallocated.
      self->pages[page_id].pid = page->pid;
      self->pages[page_id].memory_addr = page->memory_addr.copy(&page->memory_addr);
      tmp->id = page_id;
      self->move_back(self, tmp);
      return !hit;
    } else if (self->pages[tmp->id].equal(&self->pages[tmp->id], page)) {
      hit = true;
      self->move_back(self, tmp);
      return !hit;
    }
    tmp = tmp->prev;
  }

  tmp = self->front_replacement_values;
  self->pages[tmp->id].pid = page->pid;
  self->pages[tmp->id].memory_addr.free(&self->pages[tmp->id].memory_addr);
  self->pages[tmp->id].memory_addr = page->memory_addr.copy(&page->memory_addr);
  self->move_back(self, tmp);
  return !hit;
}

// True if fault occurs. False if no fault occurs
bool FIFO(struct Paging *self, struct PidToPageFrames *page) {
  bool hit = false;

  if (self->FIFO_counter >= self->num_of_pages) {
    self->FIFO_counter = 0;
  }

  struct ReplacementValueNodes *tmp = self->front_replacement_values;
  while (tmp != NULL) {
    if (tmp->id == -1) { // If id = -1, this will ALWAYS be a page fault. This means we are at the end of allocated memory. All memory after this is unallocated.
      self->pages[self->FIFO_counter].pid = page->pid;
      self->pages[self->FIFO_counter].memory_addr = page->memory_addr.copy(&page->memory_addr);
      tmp->id = self->FIFO_counter;
      break;
    } else if (self->pages[tmp->id].equal(&self->pages[tmp->id], page)) {
      hit = true;
      return !hit;
    }
    tmp = tmp->next;
  }
  tmp = self->front_replacement_values;

  if (tmp->id != -1) {
    self->pages[self->FIFO_counter].pid = page->pid;
    self->pages[self->FIFO_counter].memory_addr.free(&self->pages[self->FIFO_counter].memory_addr);
    self->pages[self->FIFO_counter].memory_addr = page->memory_addr.copy(&page->memory_addr);
  }

  self->FIFO_counter++;
  return !hit;
}

struct Paging malloc_frames(size_t num_of_pages, int lookahead_window_size, int min_pool_size, int max_pool_size, char replacement_strategy) {
  struct PidToPageFrames *pages = malloc(sizeof(struct PidToPageFrames) * num_of_pages);
  int *ids_used = malloc(sizeof(int) * num_of_pages); 

  for (size_t i = 0; i < num_of_pages; i++) {
    ids_used[i] = -1;
    pages[i] = malloc_pid_to_page_frames(BUFFER_SIZE);
    pages[i].pid = -1;
    pages[i].memory_addr.set(&pages[i].memory_addr, "EMPTY");
  }

  struct ReplacementValueNodes *front_replacement_values = NULL;
  struct ReplacementValueNodes *back_replacement_values = NULL;
  front_replacement_values = malloc(sizeof(struct ReplacementValueNodes));
  front_replacement_values->prev = NULL;
  back_replacement_values = front_replacement_values;
  for (size_t i = 0; i < num_of_pages - 1; i++) {
    back_replacement_values->id = -1;
    back_replacement_values->lfu_count = 0;
    back_replacement_values->next = malloc(sizeof(struct ReplacementValueNodes));
    struct ReplacementValueNodes *tmp = back_replacement_values;
    back_replacement_values = back_replacement_values->next;
    back_replacement_values->prev = tmp;
  }
  back_replacement_values->id = -1;
  back_replacement_values->next = NULL;

  bool (*request)(struct Paging *self, struct PidToPageFrames *page);
  char replacement_strategy_char;
  switch (replacement_strategy) {
    case SET_STRATEGY_FIFO:
      request = FIFO;
      replacement_strategy_char = SET_STRATEGY_FIFO;
      break;
    case SET_STRATEGY_LRU:
      request = LRU;
      replacement_strategy_char = SET_STRATEGY_LRU;
      break;
    case SET_STRATEGY_LFU:
      request = LFU;
      replacement_strategy_char = SET_STRATEGY_LFU;
      break;
    default:
      request = FIFO;
      replacement_strategy_char = SET_STRATEGY_FIFO;
      break;
  }

  struct Paging frames = {
    .pages = pages,
    .front_replacement_values = front_replacement_values,
    .back_replacement_values = back_replacement_values,
    .num_of_pages = num_of_pages,
    .lookahead_window_size = lookahead_window_size,
    .min_pool_size = min_pool_size,
    .max_pool_size = max_pool_size,
    .request = request,
    .print = print_frames,
    .free = free_frames,
    .move_back = move_back_paging_replacement_node,
    .replacement_strategy = replacement_strategy_char,
    .ids_used = ids_used,
    .FIFO_counter = 0,
  };

  return frames;
}

struct PageFaultsPerProcess {
  int pid;
  unsigned int num_of_page_faults;
  void (*print)(struct PageFaultsPerProcess *self);
};

void print_page_faults_per_process(struct PageFaultsPerProcess *self) {
  printf("pid: %d, number of page faults: %d", self->pid, self->num_of_page_faults);
}

struct PageFaultsPerProcess construct_page_faults_per_process() {
  struct PageFaultsPerProcess tmp = {
    .num_of_page_faults = 0,
    .print = print_page_faults_per_process,
  };

  return tmp;
}

void single_threaded_virtualization_of_memory(struct MainState main_state, char replacement_strategy, bool verbose) {
  struct Paging frames = malloc_frames(main_state.total_number_of_page_frames, main_state.lookahead_window_size, main_state.min_pool_size, main_state.max_pool_size, replacement_strategy);
  printf("===================================================================\nSingle threaded run\n");
  struct PageFaultsPerProcess *num_of_page_faults_per_process = malloc(sizeof(struct PageFaultsPerProcess) * main_state.num_of_processes);

  for (size_t i = 0; i < main_state.size_of_working_set; i++) {
    if (verbose)
      frames.print(&frames);
    struct PidToPageFrames *pid_frame = &main_state.working_set[i];
    int pid = main_state.working_set[i].pid;
    struct String *memory_addr = &main_state.working_set[i].memory_addr;

    if (pid_frame->is_hex(pid_frame)) { // normal run
      if (frames.request(&frames, pid_frame)) {
        for (size_t j = 0; j < main_state.num_of_processes; j++) {
          if (num_of_page_faults_per_process[j].pid == pid) {
            num_of_page_faults_per_process[j].num_of_page_faults++;
            break;
          }
        }
      } else if (verbose) {
        printf("\n^&^&^&^&^&^&^&^&^&^&*^&^&^&^&^&\n");
        printf("Process %d had a page hit!", pid);
        printf("\n^&^&^&^&^&^&^&^&^&^&*^&^&^&^&^&\n");
      }
    } else if (!pid_frame->is_hex(pid_frame) && atoi(memory_addr->_buf) == -1) { // This is perform at the VERY end of the process lifecycle. 
                                                                                 // TODO: free all frames with related pid
    } else { // performed at start of process lifecycle.
      num_of_page_faults_per_process[i] = construct_page_faults_per_process();
      num_of_page_faults_per_process[i].pid = pid;
      num_of_page_faults_per_process[i].num_of_page_faults = 0;
    }
  }

  if (verbose)
    frames.print(&frames);
  size_t total_number_of_page_faults = 0;
  printf("----------------------------------------------------\n");
  printf("Number of page faults per process: [\n");
  for (size_t i = 0; i < main_state.num_of_processes - 1; i++) {
    printf("\t{ ");
    num_of_page_faults_per_process[i].print(&num_of_page_faults_per_process[i]);
    total_number_of_page_faults += num_of_page_faults_per_process[i].num_of_page_faults;
    printf(" },\n");
  }
  printf("\t{ ");
  num_of_page_faults_per_process[main_state.num_of_processes - 1].print(&num_of_page_faults_per_process[main_state.num_of_processes - 1]);
  printf(" }\n]\n");
  printf("There are a total number of %zu page faults\n", total_number_of_page_faults);
  printf("At the beginning of the program, the min working size was 0. This is always true.\n");
  printf("----------------------------------------------------\n");
  printf("End of singled threaded run\n===================================================================\n\n\n\n\n");
  frames.free(&frames);
  free(num_of_page_faults_per_process);
}

void multi_threaded_virtualization_of_memory(struct MainState main_state, char replacement_strategy, bool verbose) {
  // We need semaphors to properly communicate with parent and children
  int sid;
  sid = semget(123, main_state.num_of_processes, IPC_CREAT | 0666);

  struct sembuf sb[2]; // used to control the semaphore somewhat like the 2 size array for a pipe
  sb[0].sem_num = 0; // selecting semaphore 0
  sb[0].sem_op = -1;//decrement semaphore by 1 (wait) sb[0] will be our wait operation
  sb[0].sem_flg = 0;
  sb[1].sem_num = 0;
  sb[1].sem_op =  1;//increment semaphore by 1 (release) sb[1] will be our release operation
  sb[1].sem_flg = 0;

  semop(sid,&sb[1],1); // default value of a semaphore is 0 (waiting for a release) so you have to increment it by one like here

  int shmid;
  struct PidToPageFrames *mem_msg;
  shmid = shmget(321, sizeof(struct PidToPageFrames) * main_state.num_of_processes, IPC_CREAT | 0666);

  if (shmid < 0) {
    printf("shmid failed!\n");
    return;
  }

  mem_msg = (struct PidToPageFrames *)shmat(shmid, NULL, 0);
  mem_msg->memory_addr = malloc_string(BUFFER_SIZE);

  // Start forking
  int pid = -1;
  int pnum = -1;
  for (size_t i = 0; i < main_state.num_of_processes; i++) {
    pid = fork();
    
    if (pid == 0) {
      pnum = i;
      break;
    }
  }

  if (pnum == -1) { // main fork
    struct Paging frames = malloc_frames(main_state.total_number_of_page_frames, main_state.lookahead_window_size, main_state.min_pool_size, main_state.max_pool_size, replacement_strategy);
    struct PageFaultsPerProcess *num_of_page_faults_per_process = malloc(sizeof(struct PageFaultsPerProcess) * main_state.num_of_processes);
    sb[0].sem_num = 1;
    sb[0].sem_op = -1;

    for (size_t i = 0; i < main_state.size_of_working_set; i++) {
      semop(sid, &sb[0], 1);

      // The idea was to get mem_msg after a process put something in there, then process the info, but I couldn't get semaphores to work. All I need is a d anyways. I'm done. Thanks TAs.
      if (mem_msg->is_hex(mem_msg)) {
        if (frames.request(&frames, mem_msg)) {
          for (size_t j = 0; j < main_state.num_of_processes; j++) {
            if (num_of_page_faults_per_process[j].pid == pid) {
              num_of_page_faults_per_process[j].num_of_page_faults++;
              break;
            }
          }
        } else if (verbose) {
          printf("\n^&^&^&^&^&^&^&^&^&^&*^&^&^&^&^&\n");
          printf("Process %d had a page hit!", pid);
          printf("\n^&^&^&^&^&^&^&^&^&^&*^&^&^&^&^&\n");
        }
      } else if (!mem_msg->is_hex(mem_msg) && atoi(mem_msg->memory_addr._buf) == -1) { // This is perform at the VERY end of the process lifecycle. 
                                                                                       // TODO: free all frames with related pid
      } else { // performed at start of process lifecycle.
        num_of_page_faults_per_process[i] = construct_page_faults_per_process();
        num_of_page_faults_per_process[i].pid = pid;
        num_of_page_faults_per_process[i].num_of_page_faults = 0;
      }
    }
    if (verbose)
      frames.print(&frames);
    size_t total_number_of_page_faults = 0;
    printf("----------------------------------------------------\n");
    printf("Number of page faults per process: [\n");
    for (size_t i = 0; i < main_state.num_of_processes - 1; i++) {
      printf("\t{ ");
      num_of_page_faults_per_process[i].print(&num_of_page_faults_per_process[i]);
      total_number_of_page_faults += num_of_page_faults_per_process[i].num_of_page_faults;
      printf(" },\n");
    }
    printf("\t{ ");
    num_of_page_faults_per_process[main_state.num_of_processes - 1].print(&num_of_page_faults_per_process[main_state.num_of_processes - 1]);
    printf(" }\n]\n");
    printf("There are a total number of %zu page faults\n", total_number_of_page_faults);
    printf("At the beginning of the program, the min working size was 0. This is always true.\n");
    printf("----------------------------------------------------\n");
    printf("End of singled threaded run\n===================================================================\n\n\n\n\n");
    frames.free(&frames);
    free(num_of_page_faults_per_process);
  } else { // child forks
    sb[0].sem_num = 0;
    sb[1].sem_num = 0;
    int process_num = main_state.process_nums[pnum];
    for (size_t i = 0; i < main_state.size_of_working_set; i++) {
      struct PidToPageFrames *pid_page = &main_state.working_set[i];
      if (pid_page->pid == process_num) {
        semop(sid, &sb[0], 1);
        mem_msg->pid = pid_page->pid;
        mem_msg->memory_addr = pid_page->memory_addr.copy(&pid_page->memory_addr);
        semop(sid, &sb[1], 1);
      }
    }

    exit(0); // Free's memory child has made automagically :)
  }

  sleep(1); // just make sure frees occur after the application is through
  shmdt(mem_msg);
  shmctl(shmid, 0, IPC_RMID);
  semctl(sid, 0, IPC_RMID);
}

int main(int argc, char **argv) {
  struct MainState main_state = process_input(argv[1]);

  bool verbose = false;
  if (argc >= 3 && argv[2][0] != 'n') {
    verbose = true;
  }

  if (!(argc >= 3 && argv[2][0] == 'n' && !verbose)) {
    printf("Do you want to launch this program verbosely? [y/n]");
    char yn;
    scanf("%c", &yn);
    if (yn == 'y' || yn == 'Y') {
      verbose = true;
    }
  }

  // I couldn't get multithreading to work the way I wanted it to. Atleast I got the different strategies working!!!
  printf("\nFIFO\n");
  single_threaded_virtualization_of_memory(main_state, SET_STRATEGY_FIFO, verbose);
  //multi_threaded_virtualization_of_memory(main_state, SET_STRATEGY_FIFO, verbose);
  printf("\nLRU\n");
  single_threaded_virtualization_of_memory(main_state, SET_STRATEGY_LRU, verbose);
  //multi_threaded_virtualization_of_memory(main_state, SET_STRATEGY_LRU, verbose);
  printf("\nLFU\n");
  single_threaded_virtualization_of_memory(main_state, SET_STRATEGY_LFU, verbose);
  //multi_threaded_virtualization_of_memory(main_state, SET_STRATEGY_LFU, verbose);
  return 0;
}
