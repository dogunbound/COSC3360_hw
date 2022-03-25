#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <sstream>



namespace helperFunctions {
  std::string trim(const std::string &s)
  {
    if (s.empty()) return s;
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
  unsigned int deadline;
  unsigned int computationTime;
  std::string *queue;
};

ProcessState initProcessState(const std::list<std::string> &processInfo) {
  for (std::string i : processInfo) {
    i = trim(i);
    std::cout << i << "\n";
  }

  ProcessState pState = {
  };

  return pState;
}

struct State{
  const unsigned int numOfResources;
  const unsigned int numOfProcesses;
  const unsigned int *available;
  unsigned int **maxes;
  const ProcessState *pStates;
};

void printState(State state) {
  std::cout << "Number of resources: " << state.numOfResources << "\nNumber of processes: " << state.numOfProcesses << "\n\nAvailable instances per resources:\n";
  for (unsigned int i = 0; i < state.numOfResources; i++) {
    std::cout << "R" << i + 1 << ": " << state.available[i] << std::endl;
  }

  std::cout << "\nNumber of instances per resource allowable per process:\n";
  for (unsigned int i = 0; i < state.numOfProcesses; i++) {
    std::cout << "P" << i + 1 << " maxes: ";
    for (unsigned int j = 0; j < state.numOfResources; j++) {
      std::cout << "R" << j + 1 << ": " << state.maxes[i][j] << ((j + 1 == state.numOfResources) ? "\n" : ", ");
    }
  }
}

State initState(const std::vector<std::string> &instructions) {
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
  const unsigned int numOfResources = std::stoi(instructionList.front());
  popInstructionListUntilNonEmptyLine(line, instructionList);
  const unsigned int numOfProcesses = std::stoi(instructionList.front());
  popInstructionListUntilNonEmptyLine(line, instructionList);

  popInstructionListUntilNonEmptyLine(line, instructionList);

  // Find available list
  std::list<unsigned int> availableList;
  std::vector<std::string> splitStr;
  splitStr = split(line, ' ', splitStr);
  for (auto &i : splitStr) {
    availableList.push_back(std::stoi(trim(i)));
  }
  unsigned int *available = new unsigned int[numOfResources];
  std::copy(availableList.begin(), availableList.end(), available);

  popInstructionListUntilNonEmptyLine(line, instructionList);

  // get the per process max instance allocations per resource
  unsigned int **maxes;
  maxes = new unsigned int*[numOfProcesses];
  for (unsigned int i = 0; i < numOfProcesses; i++) {
    std::vector<std::string> splitStr;
    splitStr = split(line, ' ', splitStr);
    maxes[i] = new unsigned int[numOfResources];
    for (unsigned int j = 0; j < numOfResources; j++) {
      maxes[i][j] = std::stoi(trim(splitStr[j]));
    }
    popInstructionListUntilNonEmptyLine(line, instructionList);
  }

  // Fetch process per process data and put it straight into initProcessState
  ProcessState *pStates = new ProcessState[numOfProcesses];
  for (unsigned int i = 0; i < numOfProcesses; i++) {
    std::list<std::string> processInfo;
    do {
      processInfo.push_back(line);
      popInstructionListUntilNonEmptyLine(line, instructionList);
    } while(line.find("end") == std::string::npos);
    processInfo.push_back(line);
    popInstructionListUntilNonEmptyLine(line, instructionList);

    pStates[i] = initProcessState(processInfo);
  }
  // init state struct
  State state = {
    numOfResources,
    numOfProcesses,
    available,
    maxes
  };
  return state;
}

int main(int argc, char **argv) {
  std::cout << "Program should be called like so: ./program_name sample_matrix.txt sample_words.txt\n\n";
  std::vector<std::string> instructions = readFile(argv[1]);
  std::vector<std::vector<std::string>> resources = getResourcesAsMatrix(readFile(argv[2]));
  State state = initState(instructions);
  printState(state);
  return 0;
}
