# Use a lightweight Linux base image
FROM debian:latest

# Update the package list and install needed packages
RUN apt-get update && apt-get install -y \
    iproute2 \
    make \
    gcc \
    g++ \
    nano \
    curl \
    && apt-get clean

# Set working directory (optional)
WORKDIR /app

# Copy your code into the container (optional)
COPY . .

# Default command (optional)
CMD ["/bin/bash"]

