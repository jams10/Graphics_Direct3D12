#pragma once

#include <Windows.h>
#include <cstdint> // uint32_t Ÿ�� ����� ����.

/// <summary>
/// ���ø����̼� Ŭ����.
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
	HINSTANCE	m_hInst;  // �ν��Ͻ� �ڵ�.
	HWND		m_hWnd;   // ������ �ڵ�.
	uint32_t	m_Width;  // ������ �ʺ�.
	uint32_t	m_Height; // ������ ����.
};