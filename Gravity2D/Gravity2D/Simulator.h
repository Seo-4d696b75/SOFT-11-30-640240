#pragma once

class Simulator {

	private:
	struct Star* stars;
    int original_size;
	int size;
    int cnt;
	const double dt = 1.0;
    const double unit = 10.0;
    int w, h;

    private:
    bool IsAnyStarOnScreen();
    void OnDraw();

	public:
	Simulator(int argc, char **argv, int w, int h);
	bool Update();
	~Simulator();

};