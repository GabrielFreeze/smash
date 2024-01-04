<h1>Smash</h1><img src="https://raw.githubusercontent.com/GabrielFreeze/smash/main/Documentation/wordart.png" alt="Word Art: 'SMASH'" width="1080" height="300">
<p>Welcome to Smash, a shell implementation with features inspired by Bash. This README provides an overview of the shell's design, functionality, and limitations.</p><h2>Table of Contents</h2><ul><li><a target="_new" href="#introduction">Introduction</a></li><li><a target="_new" href="#installation">Installation</a></li><li><a target="_new" href="#usage">Usage</a></li><li><a target="_new" href="#features">Features</a></li><li><a target="_new" href="#limitations">Limitations</a></li><li><a target="_new" href="#design-structure">Design Structure</a></li><li><a target="_new" href="#internal-process-flow">Internal Process Flow</a></li><li><a target="_new" href="#error-handling">Error Handling</a></li></ul><h2>Introduction</h2><p>Smash is a Bash-like shell that provides a set of features for command execution, variable handling, redirects, and more. This README aims to guide you through its usage and internals.</p><h2>Installation</h2><p>To compile Smash, navigate to the smash folder and type the following commands:</p><pre><div class="bg-black rounded-md"><div class="p-4 overflow-y-auto"><code class="!whitespace-pre hljs language-bash">make
./smash
</code></div></div></pre><p>For a video presentation demonstrating code and features, check <a target="_new" href="https://drive.google.com/file/d/1f0WcD_KkJCcQujM76XKc8HkSopTeTjU9/view?usp=sharing">this link</a>.</p><h2>Usage</h2><p>After compilation, run <code>./smash</code> to launch the Smash shell.</p><h2>Features</h2><ul><li>Input from terminal using the linenoise library.</li><li>Tokenization of input strings with various conditions.</li><li>Variable expansion and assignment.</li><li>Handling of shell and environment variables.</li><li>Redirects for internal and external commands.</li><li>Pipe support for command chaining.</li><li>Graceful program exit with memory deallocation.</li></ul><h2>Limitations</h2><ul><li>Specific order of tokens for redirects, pipes, and arguments.</li><li>Source reads a maximum of BUFSIZ characters per line.</li><li>Directory stack has a limit of STACK_SIZE different strings.</li><li>Various path-related truncations to BUFSIZE, TOKEN_SIZE, etc.</li><li>Nested source statements are not supported.</li><li>Variable assignment statements' syntax errors affect previous assignments.</li><li>Limits on pipe characters and variable assignment statements.</li></ul><h2>Design Structure</h2><p>The program follows a high-level flowchart for processing input, tokenizing, expanding variables, and executing commands. Refer to the provided flowchart for a visual representation.</p><h2>Internal Process Flow</h2><table><thead><tr><th>Process</th><th>Description</th></tr></thead><tbody><tr><td>Input</td><td>Smash takes input from the terminal using the linenoise library.</td></tr><tr><td>Tokenization</td><td>Input strings must adhere to specific conditions for tokenization, ensuring proper parsing.</td></tr><tr><td>Variable Expansion</td><td>Variables are identified during tokenization, and their locations are saved for efficient expansion.</td></tr><tr><td>Shell and Env Variables</td><td>Shell variables are stored in a linked list with counterparts for environment variables.</td></tr><tr><td>Redirects</td><td>Two independent systems handle redirects for internal and external commands. Redirect information is stored in structs.</td></tr><tr><td>Pipes</td><td>Pipes are implemented by forking child processes for each command in the pipe chain. Parent processes manage them.</td></tr><tr><td>Exiting from the Program</td><td>Graceful program exit involves deallocating all memory on the heap before termination. Exit_program flag ensures cleanup.</td></tr></tbody></table><h2>Error Handling</h2><p>Custom error messages are provided for various errors, and a global error variable is used to propagate errors through the program. System call errors trigger perror, while other errors redirect the program flow to the cleaning phase.</p><p>For detailed information and test cases, refer to the <a href="https://github.com/GabrielFreeze/smash/tree/main/Documentation">official documentation</a>.</p>


