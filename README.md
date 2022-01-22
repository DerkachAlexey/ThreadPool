# ThreadPool
Thread Pool for my own purposes

TODO: 
* Add more tests
* Think about perfect forwarding for the provided runable object and parameters of the runable object

Unfortunately, we can't use lvalue reference for provided parameters of the runable, we need to think about how to add such possibility.
However, this thread pool can be successfully used in applications.
