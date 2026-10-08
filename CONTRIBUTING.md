# Contributing

For bugs, include your exact board variant, Arduino core and library versions,
board settings, and relevant 115200-baud Serial output. State whether a problem
also occurs with another GIF and with Serial navigation (`n` / `p`).

If possible, provide a small GIF that you have permission to share, its dimensions,
and reproduction steps. Do not attach private media or credentials.

Keep changes focused. Run the host rendering tests and Arduino compilation
commands in [docs/BUILD.md](docs/BUILD.md). State what you tested on real hardware;
a passing desktop test does not verify touch, display wiring, or SD timing.

Preserve third-party license notices. Contributions to new project code are
under the root MIT license; vendored modifications retain the upstream license.
