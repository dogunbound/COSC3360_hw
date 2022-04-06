#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <sstream>
#include <tuple>
#include <queue>
#include <cstring>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 2048



namespace helperFunctions {
  std::string lowerCaseString(const std::string str) {
    std::string lowerCase = str;

    for (auto &i : lowerCase) {
      i = std::tolower(i);
    }
    
    return lowerCase;
  }
  void clearCharArrayWithNULChar(char *arr, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
      arr[i] = '\0';
    }
  }
  // https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
  std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
  }
  // https://stackoverflow.com/questions/40252753/c-converting-number-to-words
  std::string digitName(int digit);
  std::string teenName(int number);
  std::string tensName(int number);
  std::string intName(int number);

  std::vector<std::string> ones {"","one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
  std::vector<std::string> teens {"ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen","sixteen", "seventeen", "eighteen", "nineteen"};
  std::vector<std::string> tens {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

  std::string nameForNumber (long number) {
    if (number < 10) {
      return ones[number];
    } else if (number < 20) {
      return teens [number - 10];
    } else if (number < 100) {
      return tens[number / 10] + ((number % 10 != 0) ? " " + nameForNumber(number % 10) : "");
    } else if (number < 1000) {
      return nameForNumber(number / 100) + " hundred" + ((number % 100 != 0) ? " " + nameForNumber(number % 100) : "");
    } else if (number < 1000000) {
      return nameForNumber(number / 1000) + " thousand" + ((number % 1000 != 0) ? " " + nameForNumber(number % 1000) : "");
    } else if (number < 1000000000) {
      return nameForNumber(number / 1000000) + " million" + ((number % 1000000 != 0) ? " " + nameForNumber(number % 1000000) : "");
    } else if (number < 1000000000000) {
      return nameForNumber(number / 1000000000) + " billion" + ((number % 1000000000 != 0) ? " " + nameForNumber(number % 1000000000) : "");
    }
    return "error";
  }

  //https://www.geeksforgeeks.org/extract-integers-string-c/
  std::string convertAllNumbersInWordToWords(const std::string &str) {
    std::string result = str;

    std::stringstream ss;

    /* Storing the whole string into string stream */
    ss << str;

    /* Running loop till the end of the stream */
    std::string temp;
    int found;
    while (!ss.eof()) {

      /* extracting word by word from stream */
      ss >> temp;

      /* Checking the given word is integer or not */
      if (std::stringstream(temp) >> found) {
        ReplaceAll(result, std::to_string(found), nameForNumber(found));
      }

      /* To save from space at the end of string */
      temp = "";
    }

    return result;
  }
  std::string trim(const std::string &s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && isspace(*it))
      it++;

    std::string::const_reverse_iterator rit = s.rbegin();
    while (rit.base() != it && isspace(*rit))
      rit++;

    return std::string(it, rit.base());
  }
  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }

  std::vector<std::string> readFile(const std::string &file) {
    std::vector<std::string> lines;

    std::ifstream infile(file);
    std::string line;

    while(std::getline(infile, line)) {
      lines.push_back(line);
    }

    return lines;
  }
}

using namespace helperFunctions;
std::vector<std::vector<std::string>> getResourcesAsMatrix(const std::vector<std::string> &resources) {
  std::vector<std::vector<std::string>> resourceMatrix;

  for (const auto &i : resources) {
    std::vector<std::string> splitStr;
    splitStr = split(i, ',', splitStr);
    for (std::string &j : splitStr) {
      j = trim(j);
      int lastIndexOfSpace = j.find_last_of(' ');
      if (lastIndexOfSpace != -1) j = j.substr(lastIndexOfSpace+1, j.size() - 1);
    }

    resourceMatrix.push_back(splitStr);
  }

  return resourceMatrix;
}

struct ProcessState {
  int processNum;
  bool isRunning;
  int deadline;
  int computationTime;
  std::queue<std::string> queue;
  int *pipe;
};

void printProcessState(ProcessState pstate) {
  std::cout << "Process number: " << pstate.processNum << "\n" 
    << "Process deadline: " << pstate.deadline << "\n"
    << "Process computation time: " << pstate.computationTime << "\n\n"
    << "Process queue: \n";

  for (long unsigned int i = 0; i < pstate.queue.size(); i++) {
    std::string line = pstate.queue.front();
    pstate.queue.pop();

    std::cout << line << std::endl;
    pstate.queue.push(line);
  }
  std::cout << std::endl;
}

ProcessState initProcessState(std::list<std::string> &processInfo) {
  for (std::string &i : processInfo) {
    i = trim(i);
  }

  const int processNum = std::stoi(
      processInfo
      .front()
      .substr(
        processInfo.front().find_last_of('_')+1,
        processInfo.front().find_last_of(':')-1
        )
      );
  processInfo.pop_front();

  const int deadline = std::stoi(processInfo.front());
  processInfo.pop_front();
  const int computationTime = std::stoi(processInfo.front());
  processInfo.pop_front();

  std::queue<std::string> queue;
  for (const std::string &i : processInfo) {
    if (!i.empty()) queue.push(i);
  }

  int *p = new int[2];
  pipe(p);
  ProcessState pState = {
    processNum,
    true,
    deadline,
    computationTime,
    queue,
    p
  };

  return pState;
}

struct State{
  const int numOfResources;
  const int numOfProcesses;
  std::string scheduler;
  const int *available;
  int **maxes;
  ProcessState *pStates;
};

void printState(State state) {
  std::cout << "Number of resources: " << state.numOfResources << "\nNumber of processes: " << state.numOfProcesses << "\n\nAvailable instances per resources:\n";
  for (int i = 0; i < state.numOfResources; i++) {
    std::cout << "R" << i + 1 << ": " << state.available[i] << std::endl;
  }

  std::cout << "\nNumber of instances per resource allowable per process:\n";
  for (int i = 0; i < state.numOfProcesses; i++) {
    std::cout << "P" << i + 1 << " maxes: ";
    for (int j = 0; j < state.numOfResources; j++) {
      std::cout << "R" << j + 1 << ": " << state.maxes[i][j] << ((j + 1 == state.numOfResources) ? "\n" : ", ");
    }
  }

  std::cout << "\n\nProcess info:\n\n";
  for (int i = 0; i < state.numOfProcesses; i++) {
    printProcessState(state.pStates[i]);
  }
}

State initState(const std::vector<std::string> &instructions, std::string scheduler) {
  std::string line;
  std::list<std::string> instructionList(instructions.begin(), instructions.end());
  // pop instructionList until nonEmpty line is displayed
  auto popInstructionListUntilNonEmptyLine = [](std::string &line, std::list<std::string> &instructionList) {
    do {
      line = trim(instructionList.front());
      instructionList.pop_front();
    } while(line.empty() && instructionList.size() != 0);
  };

  // get number of resources and processes
  const int numOfResources = std::stoi(instructionList.front());
  popInstructionListUntilNonEmptyLine(line, instructionList);
  const int numOfProcesses = std::stoi(instructionList.front());
  popInstructionListUntilNonEmptyLine(line, instructionList);

  popInstructionListUntilNonEmptyLine(line, instructionList);

  // Find available list
  std::list<int> availableList;
  std::vector<std::string> splitStr;
  splitStr = split(line, ' ', splitStr);
  for (auto &i : splitStr) {
    availableList.push_back(std::stoi(trim(i)));
  }
  int *available = new int[numOfResources];
  std::copy(availableList.begin(), availableList.end(), available);

  popInstructionListUntilNonEmptyLine(line, instructionList);

  // get the per process max instance allocations per resource
  int **maxes;
  maxes = new int*[numOfProcesses];
  for (int i = 0; i < numOfProcesses; i++) {
    std::vector<std::string> splitStr;
    splitStr = split(line, ' ', splitStr);
    maxes[i] = new int[numOfResources];
    for (int j = 0; j < numOfResources; j++) {
      maxes[i][j] = std::stoi(trim(splitStr[j]));
    }
    popInstructionListUntilNonEmptyLine(line, instructionList);
  }

  // Fetch process per process data and put it straight into initProcessState
  ProcessState *pStates = new ProcessState[numOfProcesses];
  for (int i = 0; i < numOfProcesses; i++) {
    std::list<std::string> processInfo;
    do {
      processInfo.push_back(line);
      popInstructionListUntilNonEmptyLine(line, instructionList);
    } while(line.find("end") == std::string::npos);
    processInfo.push_back(line);
    if (instructionList.size() != 0) popInstructionListUntilNonEmptyLine(line, instructionList);

    for (auto &j : processInfo) {
      j = trim(j);
    }

    processInfo.remove("");

    pStates[i] = initProcessState(processInfo);
  }
  // init state struct
  State state = {
    numOfResources,
    numOfProcesses,
    scheduler,
    available,
    maxes,
    pStates
  };
  return state;
}

namespace taskOperations {
  int getSingleIntValueFromTask(std::string task) {
    return std::stoi(
        trim(
          task.substr(
            task.find_last_of("(") + 1,
            task.find_last_of(")") - task.find_last_of("(")- 1
            )
          )
        );
  }

  std::vector<int> getIntArrFromTask(std::string task) {
    std::vector<int> arr;

    std::vector<std::string> splitStr;
    splitStr = split(
        trim(
          task.substr(
            task.find_last_of("(") + 1,
            task.find_last_of(")") - task.find_last_of("(") - 1
            )
          ), 
        ',', splitStr
        );

    for (auto &i : splitStr) {
      i = trim(i); 
      arr.push_back(std::stoi(i));
    }

    return arr;
  }

  std::string parsedMasterString(const std::string &masterString) {
    std::string str = masterString;
    str = convertAllNumbersInWordToWords(str);
    return str;
  }

  struct StringAndCount {
    std::string str;
    int count;
  };
  std::string useResourceOnMasterString(const std::string &masterString, const std::vector<std::vector<std::string>> &resources, const int &num) {
    std::string computedMasterStr = "";

    std::vector<std::string> splitStr;
    splitStr = split(masterString, ',', splitStr);

    std::list<StringAndCount> strAndCounts;
    for (auto &i : splitStr) {
      i = trim(i);

      std::vector<std::string> countAndString;
      countAndString = split(i, ' ', countAndString);

      strAndCounts.push_back({
          trim(countAndString[1]),
          std::stoi(trim(countAndString[0]))
          });
    }

    for (const auto &i : resources) {
      for (const auto &j : i) {
        bool found = false;
        for (auto &n : strAndCounts) {
          if (n.str.find(j) != std::string::npos) {
            n.count += num;
            found = true;
          }
        }

        if (!found) {
          strAndCounts.push_back({
              j,
              num
              });
        }
      }
    }

    for (const auto &i : strAndCounts) {
      computedMasterStr += i.count + " " + i.str + ", ";
    }

    return computedMasterStr;
  }
};

struct ProgramInput {
  std::string instructionsFile;
  std::string resourcesFile;
  std::string scheduler;
};

ProgramInput parseInput(int argc, char **argv) {
  std::cout << "Program should be called like so: ./program_name --instructions=sample_matrix.txt --resources=sample_words.txt\n\n"
    << "--instructions=*.txt is your matrix file with process instructions and resource maximums\n"
    << "--resources=*.txt is your resource instances file\n"
    << "--edf for Earliest deadline first scheduler (default)\n"
    << "--llf for least laxity first scheduler\n";
  std::string instructionsFile, resourcesFile, scheduler = "edf";

  for (int i = 0; i < argc; i++) {
    if (strstr(argv[i], "--instructions=")) {
      instructionsFile = argv[i];
      instructionsFile = instructionsFile.substr(instructionsFile.find("=")+1, instructionsFile.size());
      instructionsFile = trim(instructionsFile);
    } else if (strstr(argv[i], "--resources=")) {
      resourcesFile = argv[i];
      resourcesFile = resourcesFile.substr(resourcesFile.find("=")+1, resourcesFile.size());
      resourcesFile = trim(resourcesFile);
    } else if (strstr(argv[i], "--llf")) {
      scheduler = "llf";
    } else if (strstr(argv[i], "--edf")) {
      scheduler = "edf";
    }
  }

  if (instructionsFile.empty() || resourcesFile.empty()) 
    std::cout << "\nERROR! argument instructions or resources not fullfilled!\n";

  ProgramInput input = {
    instructionsFile,
    resourcesFile,
    scheduler
  };

  std::cout << "\n\n\n" << std::endl;
  return input;
}

// I wasn't able to implement these two functions because I didn't know how to get my semaphores working right. Gave up. I hope to get extra credit for atleast attempting to implement EDF and LLF

// This is a super specific way of doing the std::sort on tuples.
// Basically the first element (unsigned int) is the process number
// second int is the comparitor weight whether it be EDF or LLF
// third int is the computation time. Results vary based on SJF or LJF is picked
auto SJF = [](std::tuple<unsigned int, int, int> &a, std::tuple<unsigned int, int, int> &b) {
  if (std::get<1>(a) == std::get<1>(b))
    return std::get<2>(a) < std::get<2>(b);
  return std::get<1>(a) < std::get<1>(b);
};

auto LJF = [](std::tuple<unsigned int, int, int> &a, std::tuple<unsigned int, int, int> &b) {
  if (std::get<1>(a) == std::get<1>(b))
    return std::get<2>(a) > std::get<2>(b);
  return std::get<1>(a) > std::get<1>(b);
};

std::vector<int> EDF(const unsigned int &numOfProcesses, ProcessState *pStates, const std::string &tieBreaker) { // pStates holds computationTime, and deadline information. 
  std::vector<int> order;
  std::vector<std::tuple<unsigned int, int, int>> deadlines(numOfProcesses);
  for (unsigned int i = 0; i < numOfProcesses; i++) {
    deadlines[i] = std::make_tuple(i, pStates[i].deadline, pStates[i].computationTime);
  }

  if (lowerCaseString(tieBreaker).find("ljf") != std::string::npos) { // LJF
    std::sort(deadlines.begin(), deadlines.end(), LJF);
  } else { // SJF
    std::sort(deadlines.begin(), deadlines.end(), SJF);
  }

  for (const auto &i : deadlines) {
    order.push_back(std::get<0>(i));
  }
  return order;
}

std::vector<int> LLF(const unsigned int &numOfProcesses, ProcessState *pStates, const std::string &tieBreaker) { // pStates holds computationTime, and deadline information. 
  std::vector<int> order;

  std::vector<std::tuple<unsigned int, int, int>> deadlines(numOfProcesses);
  for (unsigned int i = 0; i < numOfProcesses; i++) {
    deadlines[i] = std::make_tuple(i, pStates[i].deadline - pStates[i].computationTime, pStates[i].computationTime);
  }

  if (lowerCaseString(tieBreaker).find("ljf") != std::string::npos) { // LJF
    std::sort(deadlines.begin(), deadlines.end(), LJF);
  } else { // SJF
    std::sort(deadlines.begin(), deadlines.end(), SJF);
  }

  for (const auto &i : deadlines) {
    order.push_back(std::get<0>(i));
  }
  return order;
}

// I honestly don't know what I am supposed to do with this assignment. I'm lost. Atleast I have got semaphores down. This code isn't working btw.
// Best part is, i did't procrastinate. I've been working on this since the end of spring break.
// I don't even know anymore.
int main(int argc, char **argv) {
  ProgramInput input = parseInput(argc, argv);
  std::vector<std::string> instructions = readFile(input.instructionsFile);
  std::vector<std::vector<std::string>> resources = getResourcesAsMatrix(readFile(input.resourcesFile));
  State state = initState(instructions, input.scheduler);

  int sid; // semaphore
  sid = semget(123, state.numOfProcesses, IPC_CREAT | 0666);

  struct sembuf sb[2]; // used to control the semaphore somewhat like the 2 size array for a pipe
  sb[0].sem_num = 0; // selecting semaphore 0
  sb[0].sem_op = -1;//decrement semaphore by 1 (wait) sb[0] will be our wait operation
  sb[0].sem_flg = 0;
  sb[1].sem_num = 0;
  sb[1].sem_op =  1;//increment semaphore by 1 (release) sb[1] will be our release operation
  sb[1].sem_flg = 0;

  semop(sid,&sb[1],1); // default value of a semaphore is 0 (waiting for a release) so you have to increment it by one like here
                       // in order for the first process that hit's semaphore 0 to keep running.

  int shmid; // shared memory ID
  char *masterString = new char[BUFFER_SIZE];

  shmid = shmget(321, BUFFER_SIZE * sizeof(masterString), IPC_CREAT | 0666);

  if (shmid < 0) {
    std::cout << "Error with shmid" << std::endl;
    return 1;
  }

  masterString = (char *) shmat(shmid, NULL, 0);

  // Clear master string with '\0' characters
  clearCharArrayWithNULChar(masterString, BUFFER_SIZE);


  // fork hint
  int pnum = -1;
  int pid;
  for(int k =0;k<state.numOfProcesses;k++){
    pid = fork();
    if(pid ==0){
      pnum = k;
      break;
    }
  }

  using namespace taskOperations;

  if (pnum == -1) { // parent fork
    sb[0].sem_num = 0;
    sb[0].sem_op = 0;

    for (int i = 0; i < state.numOfProcesses; i++) {
      semop(sid,&sb[0],1);  // Wait for all children to finish
    }

    std::cout << "\n\n";
    for (int i = 0; i < BUFFER_SIZE; i++) {
      std::cout << masterString[i];
    }
    std::cout << std::endl;
  } else { // child forks
    ProcessState *pState = &state.pStates[pnum];
    sb[0].sem_num = 0;
    sb[1].sem_num = 0;

    std::vector<std::vector<std::string>> resourcesUsed(resources.size());
    while (!pState->queue.empty()) {
      std::string task = pState->queue.front();
      pState->queue.pop();

      if (task.find("calculate(") != std::string::npos) {
        int execTime = getSingleIntValueFromTask(task);
        pState->computationTime -= execTime;
      } else if (task.find("print_resources_used") != std::string::npos) {
        semop(sid, &sb[0], 1);
        std::cout << "\n";
        for (int i = 0; i < BUFFER_SIZE; i++) {
          std::cout << masterString[i];
        }
        std::cout << "\n" << std::endl;
        semop(sid, &sb[1], 1);
      } else if (task.find("release(") != std::string::npos) {
        std::vector<int> release = getIntArrFromTask(task);

        for (long unsigned i = 0; i < resourcesUsed.size(); i++) {
          for (int j = 0; j < release[i]; j++) {
            resourcesUsed[i].pop_back();
          }
        }

      } else if (task.find("request(") != std::string::npos) {
        std::vector<int> request = getIntArrFromTask(task);

        // Fetch. This is incorrect for now
        for (long unsigned i = 0; i < resources.size(); i++) {
          for (int j = 0; j < request[i]; j++) {
            resourcesUsed[i].push_back(resources[i][j]);
          }
        }
      } else if (task.find("use_resources(") != std::string::npos) {
        int num = getSingleIntValueFromTask(task);
        semop(sid, &sb[0], 1);
        std::string currentMasterString = masterString;
        clearCharArrayWithNULChar(masterString, BUFFER_SIZE);
        std::string newMasterString = useResourceOnMasterString(currentMasterString, resourcesUsed, num);

        for (long unsigned i = 0; i < newMasterString.size(); i++) {
          masterString[i] = newMasterString[i];
        } 
        semop(sid, &sb[1], 1);
      }
    }

    sleep(1);
    semop(sid,&sb[1],1); // wait for both processes 0 and 1 to finish before printing
    std::cout << "Finished process " << pnum + 1 << std::endl;
  }



  sleep(1); // if the semaphores are deallocated before they are finished being used will cause issues.
  shmdt(masterString);
  shmctl(shmid,0,IPC_RMID);
  semctl(sid,0,IPC_RMID);
  return 0;
}
