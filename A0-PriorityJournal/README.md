# A0: Newsroom Simulator

**Course:** Programming 2
**Term:** 2025/1
**Author:** Pietro Comin (GRR20241955)

## 📰 About the Project

This project consists of implementing a system to manage the flow of news in a newsroom. The program simulates the arrival of news and the assembly of editions, using a priority system to differentiate "breaking news" (urgent) from regular news.

The objective was to apply concepts of Abstract Data Types (ADTs) and linked list manipulation in C to create a functional data structure.

## ✔️ Features

The system is based on two main queues, one for each priority level:

* **News Registration:** The user can input the title and text of a new story, defining its priority (0 for breaking news, 1 for normal).
* **Priority Queues:** News items are inserted into the queue corresponding to their priority.
* **Selection for Edition:** When "closing an edition," the system selects the highest priority news available (breaking news always comes before regular news).
* **News Lifecycle:** News items "age" with each edition and are automatically discarded after 3 days (editions) to keep the content relevant.

## 🛠️ Technologies Used

* **Language:** C
* **Data Structure:** The system was implemented using **two queues based on linked lists**, one for each priority level, to ensure the arrival order (FIFO) within each category.

## 🚀 How to Compile and Run

The project can be compiled directly with GCC. Navigate to the project folder and run:

```bash
gcc -o jornal_sim *.c -lm
./jornal_sim
