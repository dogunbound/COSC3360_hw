#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <cmath>
#include <sstream>
#include <boost/algorithm/string.hpp>




#define BUFFER_SIZE 2048

std::vector<std::vector<bool>> readFileTo2DMatrix(std::string fileName) {
  std::vector<std::vector<bool>> m = std::vector<std::vector<bool>>();

  std::ifstream infile(fileName);
  std::string line;

  while(std::getline(infile, line)) {
    m.push_back(std::vector<bool>());
    std::vector<std::string> splitStr;
    boost::split(splitStr, line, boost::is_any_of(" "));

    for (auto i : splitStr) {
      boost::trim(i);
      if (i == "1" || i == "0") {
        m.back().push_back(i == "1");
      }
    }
  }

  return m;
}

std::list<std::string> readFileToList(std::string fileName) {
  std::set<char> delimitters = std::set<char>();
  delimitters.insert(',');
  std::list<std::string> v = std::list<std::string>();

  std::ifstream infile(fileName);
  std::string line;

  while(std::getline(infile, line)) {
    std::vector<std::string> splitStr;
    boost::split(splitStr, line, boost::is_any_of(delimitters));

    for (auto i : splitStr) {
      boost::trim(i);
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

std::string convertSmallIntToWord(int num) {
  std::string result = "";
  if (num > 99) {
    std::cout << num << " is greater than 99. Cannot convert\n";
    return result;
  }
  if (num < 0) {
    std::cout << num << " is less than 0. Cannot convert\n";
    return result;
  }

  const std::string singles[10] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
  const std::string teens[10] = { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
  const std::string tens[8] = { "twenty", "thrity", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety" };

  if (num < 10) {
    result = singles[num];
  } else if (num < 20) {
    result = teens[num - 10];
  } else {
    int tensPlace = std::floor(num / 10) - 2;
    int singlesPlace = num % 10;
    result = tens[tensPlace] + "-" + singles[singlesPlace];
  }

  return result;
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
      boost::replace_all(result, std::to_string(found), convertSmallIntToWord(found));
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


  std::set<char> delimitters;
  delimitters.insert(',');
  std::list<std::string> itemsReordered;

  for (const auto &i : items) {
    std::vector<std::string> splitStr;
    boost::split(splitStr, i, boost::is_any_of(delimitters));
    for (auto &j : splitStr) {
      boost::trim(j);
      itemsReordered.push_back(j);
    }
  }

  std::set<ItemCount> itemCount;

  delimitters = std::set<char>();
  delimitters.insert(' ');
  for (const auto &i : itemsReordered) {
    std::vector<std::string> splitStr;
    boost::split(splitStr, i, boost::is_any_of(delimitters));

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
      boost::trim(splitStr[0]);
      boost::trim(splitStr[1]);

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
