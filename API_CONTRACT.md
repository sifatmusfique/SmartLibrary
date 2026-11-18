# API contract (design, not implemented)
Names may be adapted, behavior must remain stable. Return typed status (Ok, Empty, Full, NotFound, Duplicate, Invalid, Blocked); no printing inside data structures. UI translates results. Use const accessors or visitor callbacks for traversal, do not expose mutable internals.
| Class | Required interface | Invariant |
|---|---|---|
| BookCatalogue | add, remove(id), find(id), editMetadata(id,title,author), updateStatus(id,bool), traverse | occupied prefix [0,count), unique positive IDs |
| HistoryList | append, find(txId), update(txId,title,action), remove(txId), traverse, clear | chronological order; tail next null; empty head/tail null |
| ReturnStack | push(id), pop, peek, find(id), updateAt(topOffset,id), traverse | count within capacity, LIFO |
| RequestQueue | enqueue(id), dequeue, peek, find(id), updateAt(frontOffset,id), traverse | front valid; count within capacity; FIFO |
| LibraryManager | add/delete/search/edit book, borrow, return, process/cancel head, history maintenance, stack/queue maintenance | all state changes coordinated |
## Circulation state machine
Available -> borrow -> Borrowed + history. Borrowed -> borrow request -> enqueue, no history/status change. Borrowed -> return -> Available + history + stack push. Available -> process FIFO head -> Borrowed + history + dequeue. Unavailable head -> blocked without changes. Cancel head only removes request. No automatic fulfillment on return. Delete rejects borrowed books or IDs with pending requests. New IDs positive; duplicate current IDs rejected. Reuse of deleted IDs allowed; history denotes historical snapshots. Repeated requests allowed (no student accounts). These are proposed policies, not explicit teacher mandates.
## Failure atomicity
Preflight existence, status and capacities. Allocate history node and copy snapshot before changing catalogue/stack/queue; commit with nonthrowing primitive mutations. If allocation fails, no logical state changes. Stack full rejects return until an entry is popped. Queue full rejects new request. Invalid input never mutates state. Transaction ID increment only on successful append. No arbitrary log edit changes book status.
## Complexity
Catalogue add with uniqueness/search/delete/edit/status O(n); direct index O(1). History append O(1) with tail; search/edit/delete/traverse O(h). Stack push/pop/peek O(1), search/traverse O(s), update by offset O(1). Queue enqueue/dequeue/peek O(1), search/traverse O(q), update by offset O(1). Manager borrow/return/process O(n); manager delete O(n+q); maintenance stack/queue update O(n) validation. Assume bounded string lengths; report string copy costs if unbounded. Fixed arrays allocate capacity, not just occupied slots; history O(h).
