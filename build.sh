#!/usr/bin/bash

# Name: build.sh

while [[ $# -gt 0 ]]; do
  case $1 in
    -b|--build|build)
        echo "Building project..."
        mkdir -p build  # -p prevents error if folder exists
        cd build
        cmake ..
        make
        cd .. # Return to root so next commands work correctly
      ;;
    -r|--run|run|demo)
      executable="./build/demo/csvengine_demo"
      if [ -f $executable ]; then
        echo "Running application..."
        $executable
      else
        echo "Error: Executable $executable not found. Did you build first?"
      fi
      ;;
      -t|--tests|tests)
      executable="./build/tests/run_tests"
      if [ -f $executable ]; then
        echo "Running tests..."
        $executable
      else
        echo "Error: Executable $executable not found. Did you build first?"
      fi
      ;;
    *)
      echo "Unknown option: $1"
      ;;
  esac
  shift
done
