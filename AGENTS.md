# Agent Instructions

Instructions for AI agents (opencode, anomalyco/opencode) working in this repository.

## Project

AI-generated textbook for microcontroller programming with the Embox operating
system. The repository contains documentation for Embox itself and for running
it on specific microcontrollers and development boards.

## Documentation Layout

- `docs/<arch>/<microcontroller>/<board>` — board-specific documentation,
  e.g. `docs/riscv/k1921vg1t/КФДЛ.441461.050`
- `docs/embox/` — architecture-agnostic Embox documentation

Follow this layout when adding documentation. Preserve the original board
designation (including Cyrillic characters) in directory names.

## Temporary Files

`.tmp/` is the local scratch directory for temporary files (builds, clones,
logs, experiments). It is gitignored; only `.tmp/.gitkeep` is tracked so the
directory exists after a fresh clone.

- Put disposable artifacts (toolchain downloads, cloned repositories,
  test logs) in `.tmp/`, never in the repo root or `docs/`.
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

## Workflow

- Before starting work, read relevant files under `docs/` and this file.
- When a new decision is made during a session, write it down in the
  appropriate file before the session ends.
- Verify changes (spellcheck, consistent formatting) before asking the user to review.
