# Architecture
```text
Console UI -> LibraryManager -> BookCatalogue (array)
                            -> HistoryList (singly linked)
                            -> ReturnStack (array)
                            -> RequestQueue (circular array)
```
## Models and APIs
Book { int id; string title, author; bool available; }; HistoryRecord { int id; string title, action; }; request and return entries are int book IDs.
BookCatalogue: addBook, deleteBook, searchBook, displayBooks, updateStatus; count and capacity; shift left after delete.
HistoryList: private Node { HistoryRecord data; Node* next; }; head/tail; addHistory O(1), showHistory chronological; destructor clears nodes; disable copying or implement ownership correctly.
ReturnStack: integer array, count; pushReturn, popReturn, showStack, isFull/isEmpty.
RequestQueue: integer array, front and count; insertion index (front+count)%capacity; enqueueRequest, dequeueRequest, peek, showQueue, contains; require positive capacity.
LibraryManager owns structures. Validate before mutations; allocate history node before changing status or queue/stack, or document exception guarantees and ensure no partially committed logical operations. UI parses whole lines; rejects malformed integers and handles EOF.
## Complexity (n books, h history, q requests, s returns)
Array append O(1) alone, add with duplicate check O(n); search/update by ID O(n); delete O(n); display O(n). History append O(1), traversal O(h). Stack push/pop O(1), display O(s). Queue enqueue/dequeue/peek O(1), display/contains O(q). Borrow/return O(n) with search; process O(n); delete with pending check O(n+q). Storage O(B+S+Q+h) for capacities B/S/Q; string storage additional. Complexity assumes bounded string sizes. Do not confuse primitive operations with manager costs.
## Layout
include/ for headers, src/ for implementations/main, tests/ for assertion-based tests, docs/ for diagrams and report evidence. Build with CMake and CTest; no third-party dependencies for core. Build warnings enabled. Keep GUI separate if approved.

## v2 additions (take precedence over earlier model descriptions)
HistoryRecord includes unique transactionId (64-bit counter). History list supports find/update/delete by transaction ID in O(h), maintains head/tail on deletion. Stack and queue support find first ID and update logical position; search O(s)/O(q); position update O(1) after validation; logical queue index maps to (front+position)%capacity. Catalogue metadata update O(n). Validate new IDs via catalogue O(n) at manager level. No status mutation in maintenance. Space O(B+S+Q+h) plus string data; traversal output is streamed, no copied collections needed. History node ownership is exclusive; prevent shallow copying. Capacity is fixed per instance; nonpositive capacity rejected. See API_CONTRACT.md for normative behavior.
Optional GUI: local browser HTML/CSS/JS with small C++ HTTP adapter, only after SL-006 and explicit approval. Alternative Qt if instructor prefers. No external deployment or database needed; bind local server to loopback. Browser only renders data and sends commands; C++ manager is authoritative. Serialize requests to avoid concurrent mutations. Keep console executable working independently.
