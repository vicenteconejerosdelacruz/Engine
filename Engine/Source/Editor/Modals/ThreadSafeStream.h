#pragma once
#include <mutex>
#include <sstream>
#include <string>

class ThreadSafeStream {
private:
	std::stringstream buffer;
	mutable std::mutex mtx; // Mutex para proteger el acceso

public:
	// Escribir en el stream (usado por el hilo de fondo)
	template <typename T>
	ThreadSafeStream& operator<<(const T& value) {
		std::lock_guard<std::mutex> lock(mtx);
		buffer << value;
		return *this;
	}

	// Leer todo el contenido actual como un std::string (usado por la UI/hilo principal)
	std::string str() const {
		std::lock_guard<std::mutex> lock(mtx);
		return buffer.str();
	}

	// Limpiar el stream si lo necesitas en algún momento
	void clear() {
		std::lock_guard<std::mutex> lock(mtx);
		buffer.str("");
		buffer.clear();
	}
};