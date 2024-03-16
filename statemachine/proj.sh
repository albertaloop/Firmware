# Function to build the app binary
build_app() {
  echo "Building app binary..."
    cd build
  cmake ..
  make
}

# Function to clean the build directory
clean_build() {
  echo "Removing build directory..."
    rm -rf build
}

# Function for creating the build directory
create_build_dir() {
    echo "Checking if build directory exists"
    if [ -d build ]; then
        echo "build directory already exists"
    else
        echo "Creating build directory"
        mkdir build

    fi
}


if [ -z "$1" ]; then
    create_build_dir
    build_app
else
    case "$1" in
        clean)
            clean_build
            ;;
    *)
        echo "Invalid argument: $1"
        echo "Usage: $0 [tests|clean]"
        exit 1
        ;;
    esac
fi

