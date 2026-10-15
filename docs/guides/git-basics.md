# Git Basics

The minimal Git knowledge needed to obtain and work with Embox, the NIIET
SDKs, and OpenOCD. This is not a full Git tutorial — it covers the commands
used in the bring-up and example flows.

## What Git does

Git is a distributed version-control system. A **repository** holds the
complete history of a project. **Cloning** copies a repository (including its
history) from a remote server (e.g. GitHub, GitFlic) to your machine.

```console
$ git clone <url>            # copy a repository into a new directory
$ git clone <url> <dir>      # same, into <dir>
```

After cloning, you have a **working tree** (your files), a **local copy** of
the history, and a link to the **remote** called `origin`.

## Inspecting state

```console
$ git status                 # what changed in the working tree
$ git diff                   # unstaged changes as a patch
$ git diff --cached          # changes staged for commit
$ git log --oneline          # recent commits, one line each
```

## Making changes

```console
$ git add <file>             # stage a change
$ git commit -m "<message>"  # record the staged changes in history
```

A commit is a snapshot with a message. Use short, descriptive messages in the
present tense (the Conventional Commits style is used in this repository:
`feat:`, `fix:`, `docs:`, `chore:`).

## Branches

A **branch** is an independent line of development. `main` is the default
branch; feature work happens on a separate branch and is merged back:

```console
$ git branch                 # list local branches
$ git checkout -b <branch>   # create and switch to a new branch
$ git checkout main          # switch back to main
$ git merge --no-ff <branch> # merge <branch> into the current branch
```

## Remotes

```console
$ git remote -v              # list remotes (e.g. origin)
$ git pull                   # fetch + merge changes from the remote
$ git push                   # upload local commits to the remote
```

## Useful references

- [Pro Git book](https://git-scm.com/book)
- [Git reference](https://git-scm.com/doc)
