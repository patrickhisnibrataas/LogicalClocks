# Logical Clocks Library

## Lamport Clocks

Lamport timestamps (or Lamport Clock = LC) is a simple algorithm used to determine the order of events in a distributed computer system.

If event A happened before event B then LC(A) <  LC(B), but LC(A) < LC(B) means that either event A happened before event B or it happened concurrently. This is the weakness of the Lamport Clocks.

## Vector Clocks

A Vector Clock (VC) is a vector of integers with one entry for each node in the entire distributed system.

For event A and B where VC(A) < VC(B), vector clocks guarantees that A happened before B, not later than or concurrently.

## VersionData

The VersionData class can be used to track local data in an easy way. It reacts and updates on received messages. A conflict resolution strategy can be set for when the ordering of events can not be guaranteed.

## Disclaimer

This code is not tesed beyond the unit tests written is probably buggy and should not be used :)