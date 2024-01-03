## Market book representation

[![codecov](https://codecov.io/gh/Bronek/market/graph/badge.svg?token=WyFr5ajq3O)](https://codecov.io/gh/Bronek/market)

Hi, thank you for looking at my project. Sadly the documentation is missing but that's OK since nobody is going to use it anyway.

### Idea

The basic idea was to create a market book which is:
* performant
* cache friendly
* space efficient
* suitable for both local processing and inter-process exchange

The result is a class which implements *behaviours* of a market book, but only implements the very basic parts of its *representation*. The class is meant to be used *inherited*, where derived class handles actual data storage. An example of such class is provided in the unit tests.
