#pragma once

#include <Caribou/Topology/Grid/Internal/BaseGrid.h>

namespace caribou::topology::internal {

/**
 * Simple representation of an unidimensional (1D) Grid in space.
 *
 * ** Do not use this class directly. Use instead caribou::topology::Grid. **
 *
 * The functions declared in this class can be used with any type of grids (static grid, container grid, etc.).
 *
 * Do to so, it uses the Curiously recurring template pattern (CRTP) :
 *    https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 *
 * A Grid is a set of multiple cell entities (same-length lines in 1D, rectangles
 * in 2D and rectangular hexahedrons in 3D) aligned in the x, y and z axis.
 *
 *
 * @tparam GridType_ Type of the derived grid class that will implement the final functions.
 */
template <class GridType_>
struct BaseUnidimensionalGrid : public BaseGrid<1, GridType_>
{
    static constexpr size_t Dimension = 1;

    using GridType = GridType_;
    using Base = BaseGrid<1, GridType_>;

    using UInt = typename Base::UInt;
    using NodeIndex = typename Base::NodeIndex;
    using CellIndex = typename Base::CellIndex;
    using Dimensions = typename Base::Dimensions;
    using Subdivisions = typename Base::Subdivisions;
    using LocalCoordinates = typename Base::LocalCoordinates;
    using WorldCoordinates = typename Base::WorldCoordinates;
    using GridCoordinates = typename Base::GridCoordinates;
    using CellSet = typename Base::CellSet;

    using Base::Base;
    using Base::cell_coordinates_at;

    /** Get the node index at a given grid location. */
    inline auto
    node_index_at(const GridCoordinates & coordinates) const noexcept -> NodeIndex
    {
        return coordinates[0];
    }

    /** Get the grid location of the node at the given node index */
    inline auto
    node_coordinates_at(const NodeIndex & index) const noexcept -> GridCoordinates
    {
        return GridCoordinates (index);
    }

    /** Get the node location in world coordinates at node index */
    inline auto
    node(const NodeIndex & index) const noexcept -> WorldCoordinates
    {
        const auto & anchor_position = Self().anchor_position();
        const auto H = Self().H();

        return anchor_position + index*H;
    }

    /** Get the cell index at a given grid location. */
    inline auto
    cell_index_at(const GridCoordinates & coordinates) const noexcept -> CellIndex
    {
        // Index is generated by looking at the cells as a flatten array
        const auto & i = coordinates[0];
        return i;
    }

    /** Get the grid location of the cell at the given cell index */
    inline auto
    cell_coordinates_at(const CellIndex & index) const noexcept -> GridCoordinates
    {
        return GridCoordinates (index);
    }

private:
    inline constexpr
    const GridType &
    Self() const
    {
        return static_cast<const GridType &> (*this);
    }

};

} // namespace caribou::topology::internal
