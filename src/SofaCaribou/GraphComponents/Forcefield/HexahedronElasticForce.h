#ifndef SOFACARIBOU_GRAPHCOMPONENTS_FORCEFIELD_HEXAHEDRONELASTICFORCE_H
#define SOFACARIBOU_GRAPHCOMPONENTS_FORCEFIELD_HEXAHEDRONELASTICFORCE_H

#include <sofa/core/behavior/ForceField.h>
#include <sofa/core/topology/BaseTopology.h>
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/helper/OptionsGroup.h>
#include <SofaBaseTopology/SparseGridTopology.h>

#include <Caribou/Algebra/Matrix.h>
#include <Caribou/Algebra/Vector.h>
#include <Caribou/Geometry/Hexahedron.h>

namespace SofaCaribou::GraphComponents::forcefield {

using namespace sofa::core;
using namespace sofa::core::objectmodel;
using namespace sofa::core::behavior;
using namespace sofa::core::topology;
using sofa::defaulttype::Vec3Types;
using sofa::component::topology::SparseGridTopology;

class HexahedronElasticForce : public ForceField<Vec3Types>
{
public:
    SOFA_CLASS(HexahedronElasticForce, SOFA_TEMPLATE(ForceField, Vec3Types));

    // Type definitions
    using DataTypes = Vec3Types;
    using Inherit  = ForceField<DataTypes>;
    using VecCoord = typename DataTypes::VecCoord;
    using VecDeriv = typename DataTypes::VecDeriv;
    using Coord    = typename DataTypes::Coord;
    using Deriv    = typename DataTypes::Deriv;
    using Real     = typename Coord::value_type;

    using Mat33   = typename caribou::algebra::Matrix<3, 3, Real>;
    using Vec3   = typename caribou::algebra::Vector<3, Real>;
    using Mat2424 = typename caribou::algebra::Matrix<24, 24, Real>;
    using Vec24   = typename caribou::algebra::Vector<24, Real>;

    using Hexahedron = caribou::geometry::Hexahedron<caribou::geometry::interpolation::Hexahedron8>;

    template <typename ObjectType>
    using Link = SingleLink<HexahedronElasticForce, ObjectType, BaseLink::FLAG_STRONGLINK>;

    // Data structures

    struct GaussNode {
        Real weight;
        Real jacobian_determinant;
        caribou::algebra::Matrix<Hexahedron::gauss_nodes.size(), 3> dN_dx;
        Mat33 F = Mat33::Identity();
    };

    /// Integration method used to integrate the stiffness matrix.
    enum class IntegrationMethod : unsigned int {
        /// Regular 8 points gauss integration
        Regular = 0,

        /// Hexas are recursively subdivided into cuboid subcells and the later
        /// are used to compute the inside volume of the regular hexa's gauss points.
        /// ** Requires a sparse grid topology **
        SubdividedVolume = 1,

        /// Hexas are recursively subdivided into cuboid subcells and the later
        /// are used to add new gauss points. Gauss points outside of the boundary are ignored.
        /// ** Requires a sparse grid topology **
        SubdividedGauss = 2
    };

    // Public methods

    HexahedronElasticForce();

    void init() override;
    void reinit() override;

    void addForce(
            const MechanicalParams* mparams,
            Data<VecDeriv>& d_f,
            const Data<VecCoord>& d_x,
            const Data<VecDeriv>& d_v) override;

    void addDForce(
            const MechanicalParams* /*mparams*/,
            Data<VecDeriv>& /*d_df*/,
            const Data<VecDeriv>& /*d_dx*/) override;

    void draw(const sofa::core::visual::VisualParams* vparams) override;

    SReal getPotentialEnergy(
            const MechanicalParams* /* mparams */,
            const Data<VecCoord>& /* d_x */) const override
    {return 0;}

    void addKToMatrix(sofa::defaulttype::BaseMatrix * /*matrix*/, SReal /*kFact*/, unsigned int & /*offset*/) override;

    void computeBBox(const sofa::core::ExecParams* params, bool onlyVisible) override;

    template <typename T>
    inline
    Hexahedron make_hexa(std::size_t hexa_id, const T & x) const
    {
        auto * topology = d_topology_container.get();
        const auto &node_indices = topology->getHexahedron(hexa_id);

        std::array<caribou::geometry::Node<3>, 8> nodes;
        for (std::size_t j = 0; j < 8; ++j) {
            const auto &node_id = node_indices[j];
            nodes[j] = x[node_id];
        }

        return Hexahedron(nodes);
    }

    inline
    IntegrationMethod integration_method() const
    {
        const auto m = static_cast<IntegrationMethod> (d_integration_method.getValue().getSelectedId());
        switch (m) {
            case IntegrationMethod::Regular:
                return IntegrationMethod::Regular;
            case IntegrationMethod::SubdividedVolume:
                return IntegrationMethod::SubdividedVolume;
            case IntegrationMethod::SubdividedGauss:
                return IntegrationMethod::SubdividedGauss;
            default:
                return IntegrationMethod::Regular;
        }
    }

    inline
    std::string integration_method_as_string() const
    {
        return d_integration_method.getValue().getSelectedItem();
    }

    inline
    void set_integration_method(const IntegrationMethod & m) {
        auto index = static_cast<unsigned int > (m);
        sofa::helper::WriteAccessor<Data< sofa::helper::OptionsGroup >> d = d_integration_method;
        d->setSelectedItem(index);
    }

private:
    /** (Re)Compute the tangent stiffness matrix */
    virtual void compute_K();

protected:
    Data< Real > d_youngModulus;
    Data< Real > d_poissonRatio;
    Data< UNSIGNED_INTEGER_TYPE > d_number_of_subdivisions;
    Data< bool > d_linear_strain;
    Data< bool > d_corotated;
    Data< sofa::helper::OptionsGroup > d_integration_method;
    Link<BaseMeshTopology>   d_topology_container;
    Link<SparseGridTopology> d_integration_grid;

private:
    bool recompute_compute_tangent_stiffness = false;
    std::vector<caribou::algebra::Matrix<24, 24, Real>> p_stiffness_matrices;
    std::vector<std::vector<GaussNode>> p_quadrature_nodes;
    std::vector<Mat33> p_initial_rotation;
    std::vector<Mat33> p_current_rotation;

};

} // namespace SofaCaribou::GraphComponents::forcefield

#endif //SOFACARIBOU_GRAPHCOMPONENTS_FORCEFIELD_HEXAHEDRONELASTICFORCE_H
