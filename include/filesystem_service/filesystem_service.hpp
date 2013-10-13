#if !defined(FILESYSTEM_SERVICE_FILESYSTEM_SERVICE_HPP_)
#define FILESYSTEM_SERVICE_FILESYSTEM_SERVICE_HPP_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

namespace services {

class filesystem_service
{
private:
	/** Primary io_service that receives results */
	boost::asio::io_service & io_service_;
	/** Internal io_service for doing real work. */
	boost::asio::io_service worker_service_;
	/** Processing thread is always busy */
	boost::asio::io_service::work processing_work_;
	/** Worker thread that is running background tasks */
	boost::thread worker_thread_;
	void thread_proc()
	{
		worker_service_.run();
	}
	/**
	 * Synchronous filesysten rename wrapper that is doing real work
	 * and posts completion handler to the primary io_service.
	 * @param work Primary io_service is busy.
	 * @param source Source file path.
	 * @param destination Destination file.
	 */
	template <typename Callback>
	void rename(boost::shared_ptr<boost::asio::io_service::work> work,
		boost::filesystem::path source,
		boost::filesystem::path destination,
		Callback callback)
	{
		(void)work;
		boost::system::error_code ec;
		boost::filesystem::rename(source, destination, ec);
		io_service_.post(boost::bind(callback, ec));
	}
public:
	filesystem_service(boost::asio::io_service & io_service)
		: io_service_(io_service)
		, processing_work_(worker_service_)
		, worker_thread_(boost::bind(&filesystem_service::thread_proc, this))
	{
	}
	~filesystem_service()
	{
		worker_service_.stop();
		worker_thread_.join();
	}
	template <typename Callback>
	void async_rename(const boost::filesystem::path & source,
		const boost::filesystem::path & destination,
		Callback callback)
	{
		worker_service_.post(boost::bind(
			&filesystem_service::rename<boost::_bi::protected_bind_t<Callback> >,
			this,
			boost::make_shared<boost::asio::io_service::work>(boost::ref(io_service_)),
			source,
			destination,
			boost::protect(callback)
		));
	}
};

}

#endif
