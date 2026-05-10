#include <SDL.h>
#include "../game_object.h"

class ProgressBar : public GameObject
{
public:
    ProgressBar(Vector2 position, Vector2 size);
    ~ProgressBar();

    void on_render(SDL_Renderer* renderer)override;
    void reset()override;
    void set_progress(float value);

private:
    void set_rect(const SDL_FRect& rect);
private:
    SDL_Rect _rect{};
    float _progress = 0.0f;

    SDL_Color _bg_color{ 36, 36, 54, 255 };
    SDL_Color _fill_color{ 180, 180, 220, 255 };
};