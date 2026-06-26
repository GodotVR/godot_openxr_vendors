extends Node
class_name CPULoader

var _threads: Array[Thread] = []
var _data_mutex: Mutex = Mutex.new()

var _interval: int = -1
var _exit_requested: bool = false


func _ready() -> void:
	for i in range(OS.get_processor_count() * 2):
		var thread = Thread.new()
		thread.start(_worker_loop.bind(i))
		_threads.append(thread)


func set_interval_usec(i: int) -> void:
	_data_mutex.lock()
	_interval = i
	_data_mutex.unlock()


func _exit_tree() -> void:
	_data_mutex.lock()
	_exit_requested = true
	_data_mutex.unlock()

	for thread in _threads:
		thread.wait_to_finish()


func _worker_loop(thread_num: int) -> void:
	var payload_string: String = str(thread_num)

	var local_interval: int = -1
	var local_should_exit: bool = false
	var sync_counter: int = 0

	while true:
		_data_mutex.lock()
		local_interval = _interval
		local_should_exit = _exit_requested
		_data_mutex.unlock()

		if local_should_exit:
			break

		if local_interval < 0:
			OS.delay_usec(10 * 1000)
			continue

		if local_interval < 2:
			local_interval = 0

		for x in range(50):
			payload_string = payload_string.sha256_text()
			OS.delay_usec(local_interval)
