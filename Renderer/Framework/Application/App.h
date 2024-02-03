#pragma once

#include <Windows.h>
#include <cstdint> // uint32_t 타입 사용을 위함.

/// <summary>
/// 애플리케이션 클래스.
/// </summary>
class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	HINSTANCE	m_hInst;  // 인스턴스 핸들.
	HWND		m_hWnd;   // 윈도우 핸들.
	uint32_t	m_Width;  // 윈도우 너비.
	uint32_t	m_Height; // 윈도우 높이.
};