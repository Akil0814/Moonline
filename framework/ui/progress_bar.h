#include <SDL.h>

class ProgressBar
{
public:

    void set_progress(float value);
    void set_rect(const SDL_FRect& rect);
    void render(SDL_Renderer* renderer) const;

private:
    SDL_Rect _rect{};
    float _progress = 0.0f;

    SDL_Color _bg_color{ 36, 36, 54, 255 };
    SDL_Color _fill_color{ 180, 180, 220, 255 };
};