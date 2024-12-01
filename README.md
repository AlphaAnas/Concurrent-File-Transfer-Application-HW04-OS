# **Concurrent File Transfer Application (HW04-OS)**

A concurrent file transfer application designed for efficient file transmission between a server and multiple clients. The server can handle multiple client requests concurrently, leveraging multithreading.

---

## **How to Run the Server and Client**

### **For Compiling the Server and Client**
```bash
make all
```
Ensure you run this command on a Linux system or WSL (Windows Subsystem for Linux). Additionally, make sure you have [CMake](https://cmake.org/download/) installed.

---

### **Example Usage**

#### **For Running the Server and Client**
```bash
make server
make client
```

#### **To Transfer Files:**
1. **Start the server:**
   ```bash
   ./server
   ```

2. **Start the client:**
   Use the following command to request a file from the server and specify the number of threads to use for the download:
   ```bash
   ./client <filename> <num_threads>
   ```

---

### **Key Notes**
- **Default Port:** The default port is `8080`. You can modify this in the [server.c][server.c] file.
- **Default IP Address:** The default IP address is `127.0.0.1` (localhost). This can be changed in the `client.c` file.

---

## **Features**
1. **Concurrent File Transfer:**
   - Supports multiple client connections using multithreading.
   - Efficiently handles file transfers, with each client receiving data concurrently.

2. **Dynamic Threading:**
   - Clients can specify the number of threads to use for downloading a file.

3. **File Integrity Check:**
   - Uses a hashing mechanism (e.g., `SHA-256`) to verify the integrity of the received file.

4. **Customizable Server and Client:**
   - Change the port or IP address as needed in the respective source files.

---

## **Known Limitations**
1. **Deprecated Library Warnings:**
   - The code utilizes `<openssl/sha.h>` for hashing, which may generate deprecation warnings on some systems. Consider updating or replacing the library for future compatibility.

2. **Error Handling:**
   - If the server receives an invalid file name or if the connection is interrupted, the application does not handle these errors gracefully in its current state and closes the connection.

3. **Performance Overhead:**
   - While the multithreaded design improves concurrency, excessive threading may introduce synchronization overhead.

---

## **How to Improve**
1. **Graceful Error Handling:**
   - Add better error reporting for scenarios like invalid file names, connection failures, or file truncation.

2. **Improved Hashing:**
   - Replace deprecated libraries with modern alternatives to ensure forward compatibility.

3. **File Transfer Optimization:**
   - Implement file compression to reduce transfer time and improve performance.

4. **Cross-Platform Support:**
   - Adapt the application to run seamlessly on Windows and macOS.

5. **Dynamic Resource Management:**
   - Enhance the server to manage active threads dynamically and optimize performance.

---
