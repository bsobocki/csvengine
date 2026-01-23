#!/usr/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

STATUS=0
EXIT_SUCCESS_MESSAGE=""
EXIT_FAIL_MESSAGE=""

function echoSuccess() {
  printf "${GREEN}✓ %s${NC}\n" "$1"
}

function echoError() {
  printf "${RED}✗ Error: %s${NC}\n" "$1"
}

function do_clean() {
  echo "┌──────────────────────┐"
  echo "│ Cleaning project...  │"
  echo "└──────────────────────┘"

  if [ -d "build" ]; then
    # This command deletes everything in 'build' EXCEPT the '_deps' folder (where GTest lives)
    # to avoid downloading and building it again each time
    find build -mindepth 1 -maxdepth 1 -not -name "_deps" -exec rm -rf {} +
    if [ $? -ne 0 ]; then
      echoError "Fail on cleaning 'build' directory without google tests removal."
    else
      echoSuccess "Clean. (Kept Google Test in build/_deps)"
    fi
  fi
}

function do_cleanall() {
  echo "┌─────────────────────────┐"
  echo "│ Cleaning everything...  │"
  echo "└─────────────────────────┘"

  if [ -d "build" ]; then
    rm -rf build
    if [ $? -ne 0 ]; then
      echoError "Fail on removing 'build' directory"
    else
      echoSuccess "Everything Clean."
    fi
  fi
}

function do_build() {
  mkdir -p build  # -p prevents error if folder exists
  cd build

  echo "┌──────────────────────┐"
  echo "│ Building project...  │"
  echo "└──────────────────────┘"
  
  cmake ..
  STATUS=$?

  if [ $STATUS -ne 0 ]; then
    cd ..
    return
  fi
  
  make
  STATUS=$?

  cd .. # Return to root so next commands work correctly
}

function do_run() {
  local executable="./build/demo/csvengine_demo"
  if [ -f $executable ]; then
    echo "┌──────────────────┐"
    echo "│ Running demo...  │"
    echo "└──────────────────┘"
    $executable
    STATUS=$?
  else
    echoError "Executable $executable not found. Did you build first?"
    echo "Run"
    echo ""
    echo "   ./go.sh build run"
    echo ""
    echo "to build and run tests."
  fi
}

function do_tests() {
  local executable="./build/tests/run_tests"
  if [ -f $executable ]; then
    cd build

    echo "┌───────────────────┐"
    echo "│ Running tests...  │"
    echo "└───────────────────┘"

    ctest --output-on-failure
    STATUS=$?

    cd ..
  else
    echoError "Executable $executable not found. Did you build first?"
    echo "Run"
    echo ""
    echo "   ./go.sh build tests"
    echo ""
    echo "to build and run tests."
  fi
}

function do_run_tests() {
  local filter="${1}*" # Run all tests for given test group or all if none provided
  local executable="./build/tests/run_tests"
  if [ -f $executable ]; then
    cd build/tests

    echo "┌──────────────────┐"
    echo "│ Running tests... │"
    echo "└──────────────────┘"

    ./run_tests --gtest_filter=$filter
    STATUS=$?

    cd ../..
  else
    echoError "Executable $executable not found. Did you build first?"
    echo "Run"
    echo ""
    echo "   ./go.sh build tests"
    echo ""
    echo "to build and run tests."
  fi
}

function do_run_benchmarks() {
  local executable="./build/benchmarks/run_benchmarks"
  if [ -f $executable ]; then
    cd build/benchmarks

    echo "┌──────────────────┐"
    echo "│ Running tests... │"
    echo "└──────────────────┘"

    BENCHMARKS_ARGS=""

    if [[ "$1" == "json" ]]; then
      BENCHMARKS_ARGS="--benchmark_out_format=json --benchmark_out=benchmark_results.json"
    elif [[ "$1" == "csv" ]]; then
      BENCHMARKS_ARGS="--benchmark_out_format=csv --benchmark_out=benchmark_results.csv"
    fi

    ./run_benchmarks $BENCHMARKS_ARGS
    STATUS=$?

    EXIT_SUCCESS_MESSAGE="Result file is available in build/benchmarks/benchmark_results.json"

    cd ../..
  else
    echoError "Executable $executable not found. Did you build first?"
    echo "Run"
    echo ""
    echo "   ./go.sh build benchmarks"
    echo ""
    echo "to build and run benchmarks."
  fi
}

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean|clean)
      do_clean
      ;;

    --clean-all|--cleanAll|--cleanall|clean-all|cleanall|cleanAll)
      do_cleanall
      ;;

    -b|--build|build)
      do_build
      ;;

    -r|--run|run|demo)
      do_run
      ;;

    --rebuild|rebuild)
      if [ -f $executable ]; then
        do_clean
      fi
      do_build
      ;;

    -t|--tests|tests)
      do_tests
      ;;

    -rt|--run_tests|run_tests)
      shift
      do_run_tests "$@"
      break
      ;;

    -benchmarks|-rb|--benchmarks|benchmarks|--run_benchmarks|run_benchmarks)
      shift
      do_run_benchmarks "$@"
      break
      ;;

    -a|--all|all)
      do_clean
      do_build
      if [ $STATUS -eq 0 ]; then
        do_run
      fi
      ;;

    *)
      echo "Unknown option: $1"
    ;;
  esac

  if [ $STATUS -ne 0 ]; then
    echoError "GO failed with status: $STATUS"
    exit $STATUS
  fi

  shift
done

echo ""
echoSuccess "GO finished successfully"
printf "$GREEN  $EXIT_SUCCESS_MESSAGE $NC\n"