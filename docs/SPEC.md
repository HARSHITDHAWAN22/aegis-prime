# Aegis Prime - Storage Engine Spec (v0.1)

## What this covers
Just the single-node storage engine for now. No networking, no multi-node stuff.
That comes later with Raft. This doc is for engine/ folder only.

## API (rough plan)

    put(key, value) -> bool
    get(key, out_value) -> bool
    remove(key) -> bool
    scan(start_key, end_key) -> iterator
    write_batch(list of ops) -> bool   // all or nothing
    snapshot() -> Snapshot    // stub for now, Raft will use this later
    restore(snapshot) -> bool // same, stub for now

write_batch needs to be atomic - either everything in the batch goes
through or nothing does. Will matter a lot later for the ledger (debit +
credit as one unit).

## WAL record format

    CRC32 (4 bytes) | seqno (8 bytes) | op (1 byte) | key_len (4) | val_len (4) | key | value

op = 0 for PUT, 1 for DELETE. Delete is a real flag/byte, not some string
value - storing a value that happens to look like a delete marker should
never be treated as an actual delete.

CRC covers the rest of the record. If CRC doesn't match on replay, just
cut the log there and throw away that record - this handles a crash
that happens mid-write.

seqno keeps going up globally, never resets. When two versions of the
same key exist, whichever has the higher seqno wins - not whichever file
is "newer" by position.

## SSTable format
Once written, never touched again (immutable). Sorted by key.

Layout: data blocks, then index block, then bloom filter block, then a
footer at the end.

Each block gets its own CRC. Footer keeps offsets to index/bloom
sections + min/max key in the file + max seqno seen in this file.

## Manifest
Basically a log of "file X added to level N" / "file Y removed" type
entries. To know current state, just replay this log.

Important: writing to manifest has to be atomic. Write to a temp file,
fsync it, then rename over the actual manifest. A flush isn't "done"
until manifest says so - skipping this step is how data ends up vanishing
on restart.

## Things that must always hold (will write tests for each)
1. If put() returns true and then it crashes - after restart, that
   write is either fully there or fully gone. Never half-done.
2. Multiple versions of same key -> get() always returns highest seqno
   one, no matter which file it's sitting in.
3. Flushed data survives restart (manifest has to be updated before
   WAL/memtable gets thrown away).
4. write_batch is all-or-nothing even if it crashes halfway through.
5. Compaction should never lose a key that's still alive, and should
   never bring back a deleted key (except maybe at the very bottom
   level where tombstones can finally get dropped).
6. scan(a, b) has to return every live key between a and b, checking
   memtable + all sstables, newest version wins.

## Concurrency (keep simple for now)
One writer at a time. Multiple readers can run alongside it.
When memtable gets swapped out for flushing, do it under a short lock -
readers should only ever see the old one or the new one, not some mix.

## Not doing yet (later specs)
- Multi-node / Raft
- Encryption, RBAC, Merkle stuff
- Compression
