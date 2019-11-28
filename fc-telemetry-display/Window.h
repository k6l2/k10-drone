#pragma once
class Window
{
public:
	static Window* create(char const* title, int w, int h);
	static void destroy(Window* w);
public:
	bool isOpen() const;
	void close();
	void clear(class Color const& c);
	void processEvent(SDL_Event const& event);
	void swapBuffer();
	v2i getSize() const;
	void setMouseGrabbed(bool value) const;
	int getRefreshRate() const;
	bool isVSyncEnabled() const;
private:
	SDL_Window* window = nullptr;
	SDL_GLContext context = nullptr;
	bool flagClose = false;
};
