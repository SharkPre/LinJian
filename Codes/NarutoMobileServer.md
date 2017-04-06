## NarutoMobileServer 池的用法

	1、每次需要的时候，先到list列表里面去找现在不需要使用的 Objects_ 列表里面去找
	2、没有的话，就自己new 一个出来 
	
	template <class T>
	class ObjectPoolAllocator2 : public ObjectAllocator<T>
	{
	public:
		ObjectPoolAllocator2(/*int maxcount = 10000,*/Mutex * pMutex=NullMutex::Instance(), Allocator *pAllocator=Allocator::Instance())
			: /*maxcount_(maxcount),count_(0),*/pAllocator_(pAllocator), pMutex_(pMutex)
		{
		}

		virtual ~ObjectPoolAllocator2()
		{
			for ( typename::std::list<T *>::iterator pos=Objects_.begin(); pos!=Objects_.end(); ++pos )
			{
				(*pos)->~T();
				pAllocator_->Deallocate(*pos);
			}
		}

		virtual T * Create()
		{
			LockGuard lock(pMutex_);
			if ( lock.Lock(true) != 0 ) {
				return NULL;
			}
	
			if ( Objects_.empty() ) 
			{
				//T * pChunk = (T *)pAllocator_->Allocate(sizeof(T));
				//if(count_ > maxcount_)
				//	return NULL;
				GT_INFO("Objects_Size=%d, T_size=%d", Objects_.size(), sizeof(T));
				void* pChunk = pAllocator_->Allocate(sizeof(T));
				if ( pChunk == NULL ) {
					return NULL;
				}
	
				T* p = new(pChunk) T;
				//count_++;
				return p;
				//Objects_.push_back(p);
			}
	
			//GT_DEBUG("allocate in pool");
			T* pObject = *(Objects_.begin());
			Objects_.pop_front();
	
			return pObject;
		}
	
		virtual void Destroy(T * pObject)
		{
			LockGuard lock(pMutex_);
			if ( lock.Lock(true) != 0 ) {
				return;
			}
		
			if ( pObject != NULL ) {
				//GT_DEBUG("return to pool");
				Objects_.push_front(pObject);
			}
		}
	private:
		//size_t maxcount_;
		//size_t count_;
		Allocator *	pAllocator_;
		Mutex *		pMutex_;
		std::list<T *>	Objects_;
	};
	
## 单例的实现 

	// method 1111 
	class server_app
	{
	protect:
		server_app();
		~server_app();
		
		stati server_app *m_server_app;
		
	public:
		static server_app *get_instance()
		{
			if ( 0 == m_server_app)
			{
				m_server_app = new server_app();
			}
			return server_app;
		}
	};

	// method 2222 
	clas server_app
	{
	public:
		server_app();
		~server_app();
		static server_app * Instance();
	}
	
	server_app * server_app::Instance()
	{
		static server_app serverApp;
		return &serverApp;
	}
	
	#define g_Server_app server_app::Instance();
	
	// g_Server_app 就是改对象的指针 单例指针 
	
## curl 的使用 

	// 使用 php_erl unique_code server_type 构造一个网址出来  然后请求该网址对应的数据 使用JsonCpp解析该字符串
		
	bool download_config(string php_url,string unique_code,string server_type)
	{
	 //初始化
	 CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
	 if(CURLE_OK != res)
	 {
		 BOOT_LOG (-1,"curl_global_init failed: %d", res);
	 }

	 CURL *easy_handle		= NULL;

	 string config_file_path = "../etc/bench_"+unique_code+".conf";
	 FILE *fp_bench		= fopen(config_file_path.c_str(), "w");

	 string		download_string;
	 string		temp_str;

	 request_url = php_url + "?uniquecode=" + unique_code + "&type=" + server_type;
	 
	 //设置easy handle
	 easy_handle = curl_easy_init();
	 //curl_easy_setopt(easy_handle, CURLOPT_URL, "http://172.24.16.133/bigpiece/api/gameapi/game.api.php?uniquecode=100030&type=battle");
	 curl_easy_setopt(easy_handle, CURLOPT_URL, request_url.c_str());
	 curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &save_file);
	 curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &download_string);
	 curl_easy_perform(easy_handle);

	 Json::Reader reader;
	 Json::Value json; 
	 if (false == reader.parse(download_string,json))
	 {
		  BOOT_LOG (-1,"download php string error");
	 }

	 if(json["bench"].isString() == false)
	 {
		  BOOT_LOG (-1,"download bench.conf php string error");
	 }

	 temp_str = json["bench"].asString();
	 fwrite(temp_str.c_str(), 1, temp_str.size(), fp_bench);

	 // 释放资源
	 fclose(fp_bench);

	 curl_easy_cleanup(easy_handle);

	 return true;
 }
	
## socket 读写多线程 分析 

	一个线程专门处理所有fd的数据读 
	一个线程专门处理所有fd的数据写 
	两个线程间 通过一个 char g_tmpbufer[102400]; 来通信 注意数据的格式 这个是一个循环队列
	当读线程发现有新的连接来了之后 需要发送一个信息给worker
	当读线程发现一个fd发来消息时，通知worker，离开 异常 都需要这样子来通知worker 
	

 