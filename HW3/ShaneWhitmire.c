#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define SET_STRATEGY_FIFO 'f'
#define SET_STRATEGY_LRU 'l'

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
};
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
  int num_of_processes;
  struct PidToPageFrames *working_set;
  int size_of_working_set;
  void (*print)(struct MainState *self);
};

void print_main_state(struct MainState *self) {
  printf("\n==========================================================\n");
  printf("Total number of page frames: %d\n", self->total_number_of_page_frames);
  printf("Page size: %d\n", self->page_size);
  printf("Number of page frames per process: %d\n", self->number_of_page_frames_per_process);
  printf("Look ahead window size: %d\n", self->lookahead_window_size);
  printf("Minimum pool size: %d\n", self->min_pool_size);
  printf("Maximum pool size: %d\n", self->max_pool_size);
  printf("Number of processes: %d\n", self->num_of_processes);

  printf("\nWorkingState:\n");
  for (size_t i = 0; i < self->size_of_working_set; i++) {
    self->working_set[i].print(&self->working_set[i]);
    printf("-----------\n");
  }
  printf("==========================================================\n");
}

struct MainState process_input(char *dir) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  size_t read;

  struct MainState main_state = {
    .print = print_main_state,
    .size_of_working_set = 0,
  };

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

    ppfs[i] = construct_pid_to_page_frames();
    ppfs[i].pid = atoi(pid);
    ppfs[i].memory_addr = frame_string;
    instructions[i].free(&instructions[i]);
  }
  main_state.working_set = ppfs;
  main_state.size_of_working_set = num_of_ppfs;

  return main_state;
}

struct ReplacementValueNodes {
  int id;
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
} Paging;

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

  printf("Replacement values: [ %d", self->front_replacement_values->id);
  struct ReplacementValueNodes *tmp = self->front_replacement_values;
  while (tmp->next != NULL) {
    printf(", %d", tmp->id);
    tmp = tmp->next;
  }
  printf(" ]\n");
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
bool LRU(struct Paging *self, struct PidToPageFrames *page) {
  bool hit = false;
  return !hit;
}

// True if fault occurs. False if no fault occurs
bool FIFO(struct Paging *self, struct PidToPageFrames *page) {
  bool hit = false;

  struct ReplacementValueNodes *tmp = self->front_replacement_values;
  while (tmp->next != NULL) {
    printf("f\n");
    if (self->pages[tmp->id].equal(&self->pages[tmp->id], page)) {
      hit = true;
      return !hit;
    } else if (tmp->id == -1) { // If id = -1, this will ALWAYS be a page fault. This means we are at the end of allocated memory. All memory after this is unallocated.
      self->pages[self->FIFO_counter].pid = page->pid;
      self->pages[self->FIFO_counter].memory_addr = page->memory_addr.copy(&page->memory_addr);
      tmp->id = self->FIFO_counter;
      break;
    }
    tmp = tmp->next;
  }
  
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

  printf("\nFIFO\n");
  single_threaded_virtualization_of_memory(main_state, SET_STRATEGY_FIFO, verbose);
  printf("\nLRU\n");
  single_threaded_virtualization_of_memory(main_state, SET_STRATEGY_LRU, verbose);
  return 0;
}
