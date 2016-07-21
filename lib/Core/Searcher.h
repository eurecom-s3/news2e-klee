//===-- Searcher.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_SEARCHER_H
#define KLEE_SEARCHER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include <vector>
#include <set>
#include <map>
#include <queue>

namespace llvm {
  class BasicBlock;
  class Function;
  class Instruction;
  class raw_ostream;
}

namespace klee {
  template<class T> class DiscretePDF;
  class ExecutionState;
  class Executor;

  extern llvm::cl::opt<bool> DebugLogMerge;

  class Searcher {
  public:
    virtual ~Searcher();

    virtual ExecutionState &selectState() = 0;

    virtual void update(ExecutionState *current,
                        const std::set<ExecutionState*> &addedStates,
                        const std::set<ExecutionState*> &removedStates) = 0;

    virtual bool empty() = 0;

    // prints name of searcher as a klee_message()
    // TODO: could probably make prettier or more flexible
    virtual void printName(llvm::raw_ostream &os) const {
      os << getName() << '\n';
    }

    virtual const std::string& getName() const = 0;

    // pgbovine - to be called when a searcher gets activated and
    // deactivated, say, by a higher-level searcher; most searchers
    // don't need this functionality, so don't have to override.
    virtual void activate() {}
    virtual void deactivate() {}

    // utility functions

    void addState(ExecutionState *es, ExecutionState *current = 0) {
      std::set<ExecutionState*> tmp;
      tmp.insert(es);
      update(current, tmp, std::set<ExecutionState*>());
    }

    void removeState(ExecutionState *es, ExecutionState *current = 0) {
      std::set<ExecutionState*> tmp;
      tmp.insert(es);
      update(current, std::set<ExecutionState*>(), tmp);
    }

    enum CoreSearchType {
      DFS,
      BFS,
      RandomState,
      RandomPath,
      NURS_CovNew,
      NURS_MD2U,
      NURS_Depth,
      NURS_ICnt,
      NURS_CPICnt,
      NURS_QC
    };

    ///LLVM style RTTI
    static bool classof(const Searcher* s) {return true;}
  };

  class DFSSearcher : public Searcher {
    std::vector<ExecutionState*> states;
    ExecutionState *currentState;
    static const std::string NAME;

  public:
    DFSSearcher() {
        currentState = NULL;
    }

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return states.empty(); }
    virtual const std::string& getName() const {return NAME;}

    ///LLVM style RTTI
    static bool classof(const Searcher* s) {return s->getName() ==  NAME;}
  };

  class BFSSearcher : public Searcher {
    std::deque<ExecutionState*> states;
    static const std::string NAME;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return states.empty(); }

    virtual const std::string& getName() const {return NAME;}

	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() ==  NAME;}
  };

  class RandomSearcher : public Searcher {
    std::vector<ExecutionState*> states;
    static const std::string NAME;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return states.empty(); }

    virtual const std::string& getName() const {return NAME;}

    ///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class WeightedRandomSearcher : public Searcher {
  public:
    enum WeightType {
      Depth,
      QueryCost,
      InstCount,
      CPInstCount,
      MinDistToUncovered,
      CoveringNew
    };

  private:
    DiscretePDF<ExecutionState*> *states;
    WeightType type;
    bool updateWeights;
    static const std::string NAME;
    
    double getWeight(ExecutionState*);

  public:
    WeightedRandomSearcher(WeightType type);
    ~WeightedRandomSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty();
    virtual void printName(llvm::raw_ostream &os) const {
      os << getName() << "::";
      switch(type) {
      case Depth              : os << "Depth\n"; return;
      case QueryCost          : os << "QueryCost\n"; return;
      case InstCount          : os << "InstCount\n"; return;
      case CPInstCount        : os << "CPInstCount\n"; return;
      case MinDistToUncovered : os << "MinDistToUncovered\n"; return;
      case CoveringNew        : os << "CoveringNew\n"; return;
      default                 : os << "<unknown type>\n"; return;
      }
    }

    virtual const std::string& getName() const {return NAME;}
    ///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class RandomPathSearcher : public Searcher {
    Executor &executor;
    static const std::string NAME;

  public:
    RandomPathSearcher(Executor &_executor);
    ~RandomPathSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty();

    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class MergingSearcher : public Searcher {
    Executor &executor;
    std::set<ExecutionState*> statesAtMerge;
    Searcher *baseSearcher;
    llvm::Function *mergeFunction;
    static const std::string NAME;

  private:
    llvm::Instruction *getMergePoint(ExecutionState &es);

  public:
    MergingSearcher(Executor &executor, Searcher *baseSearcher);
    ~MergingSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return baseSearcher->empty() && statesAtMerge.empty(); }

    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class BumpMergingSearcher : public Searcher {
    Executor &executor;
    std::map<llvm::Instruction*, ExecutionState*> statesAtMerge;
    Searcher *baseSearcher;
    llvm::Function *mergeFunction;
    static const std::string NAME;

  private:
    llvm::Instruction *getMergePoint(ExecutionState &es);

  public:
    BumpMergingSearcher(Executor &executor, Searcher *baseSearcher);
    ~BumpMergingSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return baseSearcher->empty() && statesAtMerge.empty(); }

    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class BatchingSearcher : public Searcher {
    Searcher *baseSearcher;
    uint64_t timeBudget;
    unsigned instructionBudget;

    ExecutionState *lastState;
    uint64_t lastStartTime;
    unsigned lastStartInstructions;
    static const std::string NAME;

  public:
    BatchingSearcher(Searcher *baseSearcher, 
                     uint64_t _timeBudget,
                     unsigned _instructionBudget);
    ~BatchingSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return baseSearcher->empty(); }
    virtual void printName(llvm::raw_ostream &os) const {
      os << "<" << NAME << "> timeBudget: " << timeBudget
         << ", instructionBudget: " << instructionBudget
         << ", baseSearcher:\n";
      baseSearcher->printName(os);
      os << "</" << NAME << ">\n";
    }

    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class IterativeDeepeningTimeSearcher : public Searcher {
    Searcher *baseSearcher;
    double time, startTime;
    std::set<ExecutionState*> pausedStates;
    static const std::string NAME;

  public:
    IterativeDeepeningTimeSearcher(Searcher *baseSearcher);
    ~IterativeDeepeningTimeSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return baseSearcher->empty() && pausedStates.empty(); }

    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

  class InterleavedSearcher : public Searcher {
    typedef std::vector<Searcher*> searchers_ty;

    searchers_ty searchers;
    unsigned index;
    static const std::string NAME;

  public:
    explicit InterleavedSearcher(const searchers_ty &_searchers);
    ~InterleavedSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::set<ExecutionState*> &addedStates,
                const std::set<ExecutionState*> &removedStates);
    bool empty() { return searchers[0]->empty(); }
    virtual void printName(llvm::raw_ostream &os) const {
      os << "<" << NAME << "> containing "
         << searchers.size() << " searchers:\n";
      for (searchers_ty::const_iterator it = searchers.begin(), ie = searchers.end();
           it != ie; ++it)
        (*it)->printName(os);
      os << "</" << NAME << ">\n";
    }
    virtual const std::string& getName() const {return NAME;}
	///LLVM style RTTI
	static bool classof(const Searcher* s) {return s->getName() == NAME;}
  };

}

#endif
