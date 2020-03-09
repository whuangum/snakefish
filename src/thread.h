#ifndef SNAKEFISH_THREAD_H
#define SNAKEFISH_THREAD_H

#include <sys/wait.h>
#include <unistd.h>

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "channel.h"

namespace snakefish {

class [[gnu::visibility("hidden")]] thread {
public:
  /*
   * No default constructor.
   */
  thread() = delete;

  /*
   * No copy constructor.
   */
  thread(const thread &t) = delete;

  /*
   * No copy assignment operator.
   */
  thread &operator=(const thread &t) = delete;

  /*
   * No move constructor.
   */
  thread(thread && t) = delete;

  /*
   * No move assignment operator.
   */
  thread &operator=(thread &&t) = delete;

  /*
   * Create a new snakefish thread.
   *
   * \param f The Python function this thread will execute.
   */
  explicit thread(py::function f)
      : is_parent(false), child_pid(0), started(false), alive(false),
        child_status(0), func(std::move(f)), channel(sync_channel()) {}

  /*
   * Destructor.
   */
  ~thread();

  /*
   * Start executing this thread. In other words, start executing the
   * underlying function.
   */
  void start();

  /*
   * Join this thread.
   *
   * This will block the calling thread until this thread terminates. If the
   * thread hasn't been started yet, this function will throw an exception.
   */
  void join();

  /*
   * Try to join this thread.
   *
   * This is the non-blocking version of `join()`.
   */
  void try_join();

  /*
   * Get the status of the thread.
   *
   * \returns `true` if this thread has been started and has not yet terminated;
   * `false` otherwise.
   */
  bool is_alive() { return alive; }

  /*
   * Get the exit status of the thread.
   *
   * \returns -1 if the thread hasn't been started yet. -2 if the thread hasn't
   * exited yet. -3 if the thread exited abnormally. Otherwise, the exit status
   * given by the thread is returned.
   *
   * Note that a snakefish thread is really a process. Hence the "exit status"
   * terminology.
   */
  int get_exit_status();

  /*
   * Get the output of the thread (i.e. the output of the underlying function).
   */
  py::object get_result() { return ret_val; }

private:
  /*
   * Run the underlying function and return the result to the parent.
   */
  void run();

  /*
   * Convenience function to get the `sender` out of `channel`.
   */
  sender &get_sender();

  /*
   * Convenience function to get the `receiver` out of `channel`.
   */
  receiver &get_receiver();

  bool is_parent;
  pid_t child_pid;
  bool started;
  bool alive;
  int child_status;
  py::function func;
  py::object ret_val;
  std::tuple<sender, receiver, sender, receiver> channel;
};

} // namespace snakefish

#endif // SNAKEFISH_THREAD_H