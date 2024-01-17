Web Server Performance Analysis Project
Introduction
Embark on a project to develop a lightweight web server with both thread-based and fork-based implementations, designed to handle multiple simultaneous clients. The server, supporting GET and HEAD methods, accepts IP:PORT as startup arguments and ensures robust DNS resolution for both IPv4 and IPv6. To streamline implementation and security, the server exclusively serves requests from its current directory, rejecting URLs with more than three '/'.

Experiments and Analysis
Conduct comprehensive HTTP performance tests using the 'Apache Benchmark tool' (ab). Generate 'big' and 'small' files and run tests on both thread-based and fork-based servers. Evaluate key metrics such as 'Time per request' and connection times. Explore the impact of concurrency on 'Requests per second' and derive insights into performance patterns.

Experimental Setup
Provide detailed specifications of the platform, including CPU, core count, memory, and storage type. Describe the network setup, specifying if experiments were conducted locally or across platforms. Include configurations for both server and client setups.

Results and Analysis
Present results from experiments, comparing performance between 'big' and 'small' files for threaded and forked servers. Discuss findings, emphasizing any performance variations and potential influencing factors. Reflect on concurrency-level results, highlighting observed patterns and their implications.
