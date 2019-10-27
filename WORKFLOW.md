# Project workflow guide

[YouTrack project board](https://simulationrenderer.myjetbrains.com/youtrack/agiles/104-0/105-0)

The order of the actions to complete single task:

1. Take chosen tast in project board. Mark as (in progress)
2. Create new branch for that task. Brunch name must follow the same pattern "(num)name",
    where num - is the number of the task in project board, 
    name - some descriptive name for task (one of two words).
3. Start implementing task. 
    Create pull request.
4. Complete task. 
    Move project board task to (test section).
5. Create some unit tests (if it is requered).
    Move project board task to (review section).
6. Resolve conflicts (if needed)
7. Wait for code review, discuss problems and make remarks/fixes.
8. Successfully merge to the master
9. Mark task as completed.

Master brunch must be stable. 
