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
      if [ -f "./build/demo/csvengine_demo" ]; then
        echo "Running application..."
        ./build/demo/csvengine_demo
      else
        echo "Error: Executable not found. Did you build first?"
      fi
      ;;
      -t|--tests|tests)
      if [ -f "./build/tests/run_tests" ]; then
        echo "Running tests..."
        ./build/tests/run_tests
      else
        echo "Error: Executable not found. Did you build first?"
      fi
      ;;
    *)
      echo "Unknown option: $1"
      ;;
  esac
  shift
done
