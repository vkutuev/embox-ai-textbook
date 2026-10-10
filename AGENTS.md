# Agent Instructions

Instructions for AI agents (opencode, anomalyco/opencode) working in this repository.

## Project

AI-generated textbook for microcontroller programming with the Embox operating
system. The repository contains documentation for Embox itself and for running
it on specific microcontrollers and development boards.

## Documentation Layout

- `docs/<arch>/<microcontroller>/<board>` — board-specific documentation,
  e.g. `docs/riscv/k1921vg1t/КФДЛ.441461.050`
- `docs/<arch>/<microcontroller>/` — microcontroller-level documentation
  (core, memory map, peripherals), e.g. `docs/riscv/k1921vg1t/`
- `docs/embox/` — architecture-agnostic Embox documentation
- `examples/<board>/<example>/` — runnable example sources (C code,
  Mybuild files) referenced from the board documentation

Follow this layout when adding documentation. Preserve the original board
designation (including Cyrillic characters) in directory names.

## Temporary Files

`.tmp/` is the local scratch directory for temporary files (builds, clones,
logs, experiments). It is gitignored; only `.tmp/.gitkeep` is tracked so the
directory exists after a fresh clone.

- Put disposable artifacts (toolchain downloads, cloned repositories,
  test logs) in `.tmp/`, never in the repo root or `docs/`.
- Never reference `.tmp/` as a source in documentation or agent
  instructions: `.tmp/` is disposable scratch space and disappears.
  Always cite the canonical upstream source instead (document title,
  upstream repository path/URL, or official page).
- Nothing in `.tmp/` (except `.gitkeep`) may be committed.
- Treat contents as disposable: agents may create and delete files there
  freely without asking.

## Guardrails

- Write agent instructions, skills, and documentation in English only.
- Use Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`, etc.) for commit messages.
- Session context is ephemeral: never rely on chat history to preserve
  decisions. Record decisions, conventions, and rationale in the appropriate
  documentation or instruction files.
- Never commit changes without explicit user approval.
- Do not add code comments unless asked.
- Follow existing file conventions when editing.
- Never cite `.tmp/` as a source in documentation or instructions: always
  cite the canonical upstream source (see "Temporary Files").

## Hardware Bring-Up

When working with real hardware (dev boards, debug adapters, serial
converters):

- Before the first run and before using a newly connected peripheral or
  adapter, tell the user the correct way to connect it: the required
  connection method (signals that must be wired) and the desired one
  (optional but recommended, e.g. VTref), preferably as a pin-mapping
  table based on the board documentation.
- Verify the physical wiring before concluding that hardware is at
  fault. An observed symptom (e.g. reset not working) is not proof of a
  board or silicon limitation — check the cable and the connector pinout
  first and ask the user how the device is connected.
- Mark observations that depend on a particular setup (adapter model,
  cable, host OS) as setup-specific in the documentation.

## Workflow

- Before starting work, read relevant files under `docs/` and this file.
- When a new decision is made during a session, write it down in the
  appropriate file before the session ends.
- Record new findings in experiment journals first; move them into the
  canonical guide only after they are confirmed (ideally by a repeated
  run). After new facts are established, revise the affected guide
  sections so stale claims do not remain.
- Verify changes (spellcheck, consistent formatting) before asking the user to review.
