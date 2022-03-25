#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <cmath>
#include <sstream>
#include <cctype>
#include <locale>
#include <unistd.h>
#include <list>


// https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
std::string trim(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && isspace(*it))
        it++;

    std::string::const_reverse_iterator rit = s.rbegin();
    while (rit.base() != it && isspace(*rit))
        rit++;

    return std::string(it, rit.base());
}

// https://stackoverflow.com/questions/275404/splitting-strings-in-c
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
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




#define BUFFER_SIZE 2048

std::vector<std::vector<bool>> readFileTo2DMatrix(std::string fileName) {
  std::vector<std::vector<bool>> m = std::vector<std::vector<bool>>();

  std::ifstream infile(fileName);
  std::string line;

  while(std::getline(infile, line)) {
    m.push_back(std::vector<bool>());
    std::vector<std::string> splitStr;
    splitStr = split(line, ' ', splitStr);

    for (auto i : splitStr) {
      i = trim(i);
      if (i == "1" || i == "0") {
        m.back().push_back(i == "1");
      }
    }
  }

  return m;
}

std::list<std::string> readFileToList(std::string fileName) {
  std::list<std::string> v = std::list<std::string>();

  std::ifstream infile(fileName);
  std::string line;

  while(std::getline(infile, line)) {
    std::vector<std::string> splitStr;
    splitStr = split(line, ',', splitStr);

    for (auto i : splitStr) {
      i = trim(i);
      v.push_back(i);
    }
  }

  return v;
}

void clearCharArrayWithNULChar(char *arr, unsigned int len) {
  for (unsigned int i = 0; i < len; i++) {
    arr[i] = '\0';
  }
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

struct VertexTupleToPipeId {
  std::tuple<unsigned int, unsigned int> vertTuple;
  int *pipe;
};

std::vector<std::tuple<unsigned int, unsigned int>> makeVertexTupleArray(const std::vector<std::vector<bool>> &mat) {
  std::vector<std::tuple<unsigned int, unsigned int>> fromVertToVertArr = std::vector<std::tuple<unsigned int, unsigned int>>(); 

  for (unsigned int i = 0; i < mat.size(); i++) {
    for (unsigned int j = 0; j < mat[i].size(); j++) {
      if (mat[i][j]) fromVertToVertArr.push_back(std::make_tuple(i, j));
    }
  }

  return fromVertToVertArr;
}

std::list<unsigned int> getParentNodes(const std::vector<std::vector<bool>> &mat) {
  std::vector<std::tuple<unsigned int, unsigned int>> fromVertToVertArr = makeVertexTupleArray(mat); 

  std::set<unsigned int> nonParentNodes = std::set<unsigned int>();

  for (const auto &i : fromVertToVertArr) {
    nonParentNodes.insert(std::get<1>(i));
  }
  std::list<unsigned int> parentNodes = std::list<unsigned int>();
  for (unsigned int i = 0; i < mat.size(); i++) {
    if (nonParentNodes.find(i) == nonParentNodes.end()) parentNodes.push_back(i);
  }

  return parentNodes;
}

std::vector<int*> makePipesFromAdjacencyMat(const std::vector<std::vector<bool>> &mat) {
  std::vector<int*> pipes = std::vector<int*>();

  for (const auto &i : mat) {
    for (const auto &j : i) {
      if (j) pipes.push_back(new int[2]);
    }
  }

  for (auto &i : pipes) {
    pipe(i);
  }

  return pipes;
}

std::list<VertexTupleToPipeId> makePipeAndTupleRelations(const std::vector<std::vector<bool>> &mat) {
  std::vector<std::tuple<unsigned int, unsigned int>> vertTupleArray = makeVertexTupleArray(mat);
  std::vector<int*> pipes = makePipesFromAdjacencyMat(mat);

  std::list<VertexTupleToPipeId> vertTupleAndPipeList = std::list<VertexTupleToPipeId>();

  for (int i = 0; i < pipes.size(); i++) {
    VertexTupleToPipeId tmp = {vertTupleArray[i], pipes[i]};

    vertTupleAndPipeList.push_back(tmp);
  }

  return vertTupleAndPipeList;
}

struct ItemCount {
  mutable int count;
  std::string nonPluralItem;
};

inline bool operator==(const ItemCount &a, const ItemCount &b) { return a.nonPluralItem == b.nonPluralItem; }

inline bool operator<(const ItemCount &a, const ItemCount &b) { return a.nonPluralItem < b.nonPluralItem; }

std::string removeEverythingAfterNonAlphaNumericCharacter(const std::string &str) {
  std::string result = "";

  for (const auto &i : str) {
    if (!isalnum(i)) break;

    result += i;
  }

  return result;
}

std::string combineItems(const std::list<std::string> &items) {
  std::string result = "";
  if (items.size() == 1) return items.back();

  std::list<std::string> itemsReordered;
  for (const auto &i : items) {
    std::vector<std::string> splitStr;
    splitStr = split(i, ',', splitStr);

    for (auto &j : splitStr) {
      j = trim(j);
      itemsReordered.push_back(j);
    }
  }

  std::set<ItemCount> itemCount;
  for (const auto &i : itemsReordered) {
    std::vector<std::string> splitStr;
    splitStr = split(i, ' ', splitStr);


    splitStr.back() = removeEverythingAfterNonAlphaNumericCharacter(splitStr.back());
    
    if (splitStr.back().substr(splitStr.back().length() - 1) == "s") {
      splitStr.back().resize(splitStr.back().length() - 1);
    }

    if (splitStr.size() == 1) {
      ItemCount tmp = { 1, splitStr.back() };

      if (!itemCount.insert(tmp).second) { // if insert failed, we need to find it and increment
        std::set<ItemCount>::iterator itemCountIterator = itemCount.find(tmp);
        itemCountIterator->count++;
      }
    } else {
      splitStr[0] = trim(splitStr[0]);
      splitStr[1] = trim(splitStr[1]);

      ItemCount tmp = { 
        std::stoi(splitStr[0]),
        splitStr[1]
      };

      if (!itemCount.insert(tmp).second) {
        std::set<ItemCount>::iterator itemCountIterator = itemCount.find(tmp);
        itemCountIterator->count += tmp.count;
      }
    }
  }

  for (const auto &i : itemCount) {
    result += (result.size() == 0 ? "" : ", ") + std::to_string(i.count) + " " + i.nonPluralItem + (i.count > 1 ? "s": "");
  }

  return result;
}

std::string forkingOutput(const std::vector<std::vector<bool>> &mat, const std::list<std::string> &items) {
  std::string result = "";
  std::list<unsigned int> parentNodes = getParentNodes(mat);
  std::list<VertexTupleToPipeId> pipeTuples = makePipeAndTupleRelations(mat);
  int resultPipe[2];
  pipe(resultPipe);

  std::vector<int*> checkIfInputPipes = std::vector<int*>();
  std::vector<int*> checkIfOutputPipes = std::vector<int*>();
  std::vector<int*> inputPipes = std::vector<int*>();
  std::vector<int*> outputPipes = std::vector<int*>();

  for (int i = 0; i < mat.size(); i++) {
    checkIfInputPipes.push_back(new int[2]);
    checkIfOutputPipes.push_back(new int[2]);

    pipe(checkIfInputPipes[i]);
    pipe(checkIfOutputPipes[i]);
    
    inputPipes.push_back(new int[2]);
    outputPipes.push_back(new int[2]);

    pipe(inputPipes[i]);
    pipe(outputPipes[i]);
  }


  int pid = -1;
  int pnum = -1;
  for (unsigned int i = 0; i < mat.size(); i++) {
    pid = fork();

    if (pid == 0) {
      pnum = i;
      break;
    }
  }

  if (pnum != -1) {
    unsigned int numOfChildren = 0;
    unsigned int numOfParents = 0;
    std::list<VertexTupleToPipeId> parentPipes = std::list<VertexTupleToPipeId>();
    std::list<VertexTupleToPipeId> childPipes = std::list<VertexTupleToPipeId>();
    // Iterator delete on if: https://stackoverflow.com/questions/596162/can-you-remove-elements-from-a-stdlist-while-iterating-through-it
    std::list<VertexTupleToPipeId>::iterator i = pipeTuples.begin();
    while (i != pipeTuples.end()) {
      unsigned int from = std::get<0>(i->vertTuple);
      unsigned int to = std::get<1>(i->vertTuple);

      if (from == pnum) {
        childPipes.push_back(*i);
        numOfChildren++;
        i++;
      } else if (to == pnum) {
        parentPipes.push_back(*i);
        numOfParents++;
        i++;
      } else {
        close(i->pipe[0]);
        close(i->pipe[1]);
        pipeTuples.erase(i++);
      }
    }

    for (int i = 0; i < checkIfInputPipes.size(); i++) {
      if (pnum != i) {
        close(checkIfInputPipes[i][0]);
        close(checkIfInputPipes[i][1]);
        close(checkIfOutputPipes[i][0]);
        close(checkIfOutputPipes[i][1]);
      }
    }

    std::string itemsProc;
    if (numOfParents == 0) {
      bool input = true;
      bool output = false;
      write(checkIfInputPipes[pnum][1], &input, sizeof(input));
      write(checkIfOutputPipes[pnum][1], &output, sizeof(output));
   
      close(inputPipes[pnum][1]);
      char parentItem[BUFFER_SIZE];
      clearCharArrayWithNULChar(parentItem, BUFFER_SIZE);
      read(inputPipes[pnum][0], &parentItem, sizeof(parentItem));
      close(inputPipes[pnum][0]);
      close(outputPipes[pnum][0]);
      close(outputPipes[pnum][1]);

      itemsProc = removeEverythingAfterNonAlphaNumericCharacter(std::string(parentItem));
    } else {
      if (numOfChildren == 0) {
        bool input = false;
        bool output = true;
        write(checkIfOutputPipes[pnum][1], &output, sizeof(input));
        write(checkIfInputPipes[pnum][1], &input, sizeof(output));
        close(inputPipes[pnum][1]);
        close(inputPipes[pnum][0]);
      } else {
        bool input = false;
        bool output = false;
        write(checkIfOutputPipes[pnum][1], &output, sizeof(input));
        write(checkIfInputPipes[pnum][1], &input, sizeof(output));
        close(inputPipes[pnum][1]);
        close(inputPipes[pnum][0]);
        close(outputPipes[pnum][0]);
        close(outputPipes[pnum][1]);
      }

      std::list<std::string> items;
      for (auto &i : parentPipes) {
        close(i.pipe[1]);
        char item[BUFFER_SIZE];
        clearCharArrayWithNULChar(item, BUFFER_SIZE);
        read(i.pipe[0], &item, sizeof(item));
        close(i.pipe[0]);
        items.push_back(item);
      }

      itemsProc = combineItems(items);
    }
    close(checkIfInputPipes[pnum][0]);
    close(checkIfInputPipes[pnum][1]);
    close(checkIfOutputPipes[pnum][0]);
    close(checkIfOutputPipes[pnum][1]);

    for(auto &i : childPipes) {
      close(i.pipe[0]);
      write(i.pipe[1], itemsProc.c_str(), itemsProc.size());
      close(i.pipe[1]);
    }

    if (numOfChildren == 0) {
      close(outputPipes[pnum][0]);
      write(outputPipes[pnum][1], itemsProc.c_str(), itemsProc.size());
      close(outputPipes[pnum][1]);
    }

    exit(0);
  } else if (pnum == -1) {
    std::list<std::string>::const_iterator itemIterator = items.begin();
    for (int i = 0; i < mat.size(); i++) {
      bool isInput;
      read(checkIfInputPipes[i][0], &isInput, sizeof(isInput));

      close(checkIfInputPipes[i][0]);
      close(checkIfInputPipes[i][1]);

      close(inputPipes[i][0]);
      if (isInput) {
        write(inputPipes[i][1], itemIterator->c_str(), itemIterator->size());
        itemIterator++;
      }
      close(inputPipes[i][1]);
    }

    std::list<std::string> tmpStringListForCombiningItems;
    for (int i = 0; i < mat.size(); i++) {
      bool isOutput;
      read(checkIfOutputPipes[i][0], &isOutput, sizeof(isOutput));

      close(checkIfOutputPipes[i][0]);
      close(checkIfOutputPipes[i][1]);

      close(outputPipes[i][1]);
      if (isOutput) {
        char tmpOutput[BUFFER_SIZE];
        clearCharArrayWithNULChar(tmpOutput, BUFFER_SIZE);
        read(outputPipes[i][0], tmpOutput, sizeof(tmpOutput));
        tmpStringListForCombiningItems.push_back(tmpOutput);
      }
      close(outputPipes[i][0]);
    }

    result = combineItems(tmpStringListForCombiningItems);
  }

  return result;
}

int main(int argc, char *argv[]) {
  if (argc < 3) { 
    std::cout << "Not enough arguments. Example input: ./a.out input1.txt input2.txt\n";
    return 1;
  }

  std::vector<std::vector<bool>> mat = readFileTo2DMatrix(argv[1]);
  std::list<std::string> items = readFileToList(argv[2]);

  std::string result = convertAllNumbersInWordToWords(forkingOutput(mat, items));

  std::cout << result << std::endl;
  return 0;
}
