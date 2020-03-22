/**
 * \file channel.h
 */

#ifndef SNAKEFISH_CHANNEL_H
#define SNAKEFISH_CHANNEL_H

#include <atomic>
#include <tuple>
#include <utility>

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "buffer.h"
#include "shared_buffer.h"

namespace snakefish {

static const unsigned PICKLE_PROTOCOL = 4;
static const size_t MAX_SOCK_MSG_SIZE = 1024;                          // bytes
static const size_t DEFAULT_CHANNEL_SIZE = 2l * 1024l * 1024l * 1024l; // 2 GiB

/**
 * \brief An IPC channel with built-in synchronization and (semi-automatic)
 * reference counting support.
 *
 * The support for reference counting is "semi-automatic" in the sense that
 * the `channel::fork()` function must be called right before calling
 * the system `fork()`.
 */
class channel {
public:
  /**
   * \brief No default constructor;
   */
  channel() = delete;

  /**
   * \brief Destructor implementing reference counting.
   */
  ~channel();

  /**
   * \brief Copy constructor implementing reference counting.
   */
  channel(const channel &t);

  /**
   * \brief No copy assignment operator.
   */
  channel &operator=(const channel &t) = delete;

  /**
   * \brief Move constructor implementing reference counting.
   */
  channel(channel &&t) noexcept;

  /**
   * \brief No move assignment operator.
   */
  channel &operator=(channel &&t) = delete;

  /**
   * \brief Create a pair of `channel` with buffer size `buffer_size`.
   *
   * \param buffer_size The size of the channel buffer, which is allocated as
   * shared memory.
   *
   * \returns A pair of `channel`. One for each communicating party.
   */
  friend std::pair<channel, channel> create_channel(size_t buffer_size);

  /**
   * \brief Send some bytes.
   *
   * \param bytes Pointer to the start of the bytes.
   * \param len Number of bytes to send.
   *
   * \throws e Throws `std::runtime_error` if the underlying buffer does not
   * have enough space to accommodate the request OR if some socket error
   * occurred.
   */
  void send_bytes(const void *bytes, size_t len);

  /**
   * \brief Send a python object.
   *
   * This function will serialize `obj` using `pickle` and send the binary
   * output.
   *
   * \throws e Throws `std::runtime_error` if the underlying buffer does not
   * have enough space to accommodate the request OR if some socket error
   * occurred.
   */
  void send_pyobj(const py::object &obj);

  /**
   * \brief Receive some bytes.
   *
   * \param len Number of bytes to receive.
   *
   * \returns The received bytes wrapped in a `buffer`.
   *
   * \throws e Throws `std::runtime_error` if the underlying buffer does not
   * have enough content to accommodate the request OR if some socket error
   * occurred.
   */
  buffer receive_bytes(size_t len);

  /**
   * \brief Receive a python object.
   *
   * This function will receive some bytes and deserialize them using `pickle`.
   *
   * \throws e Throws `std::runtime_error` if some socket error occurred.
   */
  py::object receive_pyobj();

  /**
   * Called by the client to indicate that this `channel` is about to be
   * shared with another process.
   */
  void fork();

protected:
  /**
   * \brief Private constructor to be used by the friend functions.
   */
  channel(const int socket_fd, shared_buffer shared_mem,
          std::atomic_uint32_t *ref_cnt, std::atomic_uint32_t *local_ref_cnt,
          const bool fork_shared_mem)
      : socket_fd(socket_fd), shared_mem(std::move(shared_mem)),
        ref_cnt(ref_cnt), local_ref_cnt(local_ref_cnt),
        fork_shared_mem(fork_shared_mem) {}

  /**
   * \brief Unix domain socket file descriptor. Used to send small messages.
   */
  int socket_fd;

  /**
   * \brief The buffer used to hold large messages.
   */
  shared_buffer shared_mem;

  /**
   * \brief Global/interprocess reference counter.
   */
  std::atomic_uint32_t *ref_cnt;

  /**
   * \brief Process local reference counter.
   */
  std::atomic_uint32_t *local_ref_cnt;

  /**
   * \brief A flag indicating whether this channel should fork its
   * `shared_buffer`.
   */
  bool fork_shared_mem;
};

/**
 * \brief Create a pair of `channel` with buffer size `DEFAULT_CHANNEL_SIZE`.
 *
 * \returns A pair of `channel`. One for each communicating party.
 *
 * \throws e Throws `std::runtime_error` if socket creation failed. Throws
 * `std::bad_alloc` if `mmap()` failed.
 */
std::pair<channel, channel> create_channel();

/**
 * \brief Create a pair of `channel` with buffer size `buffer_size`.
 *
 * \param buffer_size The size of the channel buffer, which is allocated as
 * shared memory.
 *
 * \returns A pair of `channel`. One for each communicating party.
 *
 * \throws e Throws `std::runtime_error` if socket creation failed. Throws
 * `std::bad_alloc` if `mmap()` failed.
 */
std::pair<channel, channel> create_channel(size_t buffer_size);

} // namespace snakefish

#endif // SNAKEFISH_CHANNEL_H
