#include "components.hpp"

void Tform::move()
{
    pos += vel;
}

RVector2 Sprite::size() const
{
    return sprite.GetSize();
}

void Sprite::set_pos(RVector2 pos)
{
    sprite.SetPosition(pos);
}

void BBox::sync(Tform transform)
{
    bounding_box.SetPosition(transform.pos);
    bounding_box.x += (BBOX_DEFAULT_SIZE - bounding_box.width) / 2;
    bounding_box.y += (BBOX_DEFAULT_SIZE - bounding_box.height);
}

bool BBox::collides(BBox other_bbox) const
{
    return bounding_box.CheckCollision(other_bbox.bounding_box);
}

bool BBox::x_overlaps(BBox other_bbox) const
{
    const float bbox_x = bounding_box.x;
    const float bbox_width = bounding_box.width;
    const float other_bbox_x = other_bbox.bounding_box.x;
    const float other_bbox_width = other_bbox.bounding_box.width;

    return (bbox_x >= other_bbox_x && bbox_x - other_bbox_x < other_bbox_width)
           || (other_bbox_x >= bbox_x && other_bbox_x - bbox_x < bbox_width);
}

bool BBox::y_overlaps(BBox other_bbox) const
{
    const float bbox_y = bounding_box.y;
    const float bbox_height = bounding_box.height;
    const float other_bbox_y = other_bbox.bounding_box.y;
    const float other_bbox_height = other_bbox.bounding_box.height;

    return (bbox_y >= other_bbox_y && bbox_y - other_bbox_y < other_bbox_height)
           || (other_bbox_y >= bbox_y && other_bbox_y - bbox_y < bbox_height);
}

void BBox::set_size(RVector2 size)
{
    bounding_box.SetSize(size);
}
