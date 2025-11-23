#!/usr/bin/bash

function do_clean() {
  if [ -d "build" ]; then
    # This command deletes everything in 'build' EXCEPT the '_deps' folder (where GTest lives)
    find build -mindepth 1 -maxdepth 1 -not -name "_deps" -exec rm -rf {} +
    echo "Clean. (Kept Google Test in build/_deps)"
  fi
}

function do_cleanall() {
  if [ -d "build" ]; then
    rm -rf build
    echo "Everything Clean."
  fi
}

function do_build() {
  echo "Building project..."
  mkdir -p build  # -p prevents error if folder exists
  cd build
  cmake ..
  make
  cd .. # Return to root so next commands work correctly
}

function do_run() {
  executable="./build/demo/csvengine_demo"
  if [ -f $executable ]; then
    echo "Running demo application..."
    $executable
  else
    echo "Error: Executable $executable not found. Did you build first?"
    echo "Run"
    echo " ./go.sh build run"
    echo "to build and run tests."
  fi
}

function do_tests() {
  executable="./build/tests/run_tests"
  if [ -f $executable ]; then
    echo "Running tests..."
    $executable
  else
    echo "Error: Executable $executable not found. Did you build first?"
    echo "Run"
    echo " ./go.sh build tests"
    echo "to build and run tests."
  fi
}

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean|clean)
      do_clean
      ;; 
    -b|--build|build)
      do_build
      ;;
    -r|--run|run|demo)
      do_run
      ;;
    -t|--tests|tests)
      do_tests
      ;;
    -a|--all|all)
      do_clean
      do_build
      do_run
      ;;
    *)
      echo "Unknown option: $1"
    ;;
  esac
  shift
done
