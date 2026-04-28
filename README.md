# Webserv

A custom **HTTP/1.1 web server built in C++**, inspired by the architecture of modern web servers such as NGINX.
This project focuses on **network programming, concurrency, HTTP protocol handling, and systems-level software design**.

---

## 🚀 Overview

`webserv` is a high-performance web server implemented from scratch in **C++**, designed to handle multiple client connections concurrently using **non-blocking I/O**.

The goal of the project is to recreate the core functionalities of a production-grade web server, including request parsing, routing, CGI execution, and configuration management.

---

## ✨ Features

* **HTTP/1.1 compliant server**
* Support for `GET`, `POST`, and `DELETE`
* **Multiple client connections** using `poll()`
* **Non-blocking sockets**
* Static file serving
* File uploads
* Custom error pages
* Route configuration
* Virtual hosts
* Request redirections
* **CGI support** for dynamic content
* Configurable ports and server blocks
* Robust error handling

---

## 🛠 Tech Stack

* **Language:** C++
* **Networking:** Sockets, TCP/IP
* **I/O Multiplexing:** `poll()`
* **Protocols:** HTTP/1.1
* **System Programming:** Unix/Linux APIs

---

## 🏗 Architecture

The server is built around an **event-driven architecture**:

1. **Socket initialization**

   * Create listening socket
   * Bind to configured port
   * Start listening

2. **Event loop**

   * Use `poll()` to monitor multiple sockets
   * Accept new client connections
   * Read incoming requests
   * Send responses

3. **Request lifecycle**

   * Parse raw HTTP request
   * Match route / virtual host
   * Process request method
   * Generate response
   * Send data to client

---

## 📦 Supported HTTP Methods

### GET

Used to retrieve static files or resources.

Example:

```http
GET /index.html HTTP/1.1
```

### POST

Used for form submissions and uploads.

Example:

```http
POST /upload HTTP/1.1
```

### DELETE

Used to remove server-side resources.

Example:

```http
DELETE /file.txt HTTP/1.1
```

---

## ⚙️ Configuration Example

```conf
server {
    listen 8080;
    server_name localhost;

    location / {
        root ./www;
        index index.html;
    }

    location /upload {
        methods POST;
        upload_path ./uploads;
    }

    error_page 404 ./errors/404.html;
}
```

---

## ▶️ Build & Run

```bash
make
./webserv config/default.conf
```

---

## 🧪 Testing

You can test the server with:

```bash
curl http://localhost:8080
```

Or directly from the browser:

```text
http://localhost:8080
```

---

## 📚 What I Learned

This project strengthened my understanding of:

* low-level networking
* socket programming
* HTTP protocol internals
* concurrency
* system calls
* performance optimization
* memory-safe C++ development

---

## 👨‍💻 Author

Developed as part of the **42 Network curriculum**.
