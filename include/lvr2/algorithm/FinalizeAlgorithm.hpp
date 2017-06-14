/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/*
 * FinalizeAlgorithm.hpp
 *
 *  @date 13.06.2017
 *  @author Johan M. von Behren <johan@vonbehren.eu>
 */

#ifndef LVR2_ALGORITHM_FINALIZEALGORITHM_H_
#define LVR2_ALGORITHM_FINALIZEALGORITHM_H_

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <lvr2/geometry/BaseMesh.hpp>
#include <lvr/io/MeshBuffer.hpp>

namespace lvr2
{

/**
 * @brief
 */
template<typename BaseVecT>
class FinalizeAlgorithm
{
public:
    using VertexHandle = typename BaseMesh<BaseVecT>::VertexHandle;
    using FaceHandle = typename BaseMesh<BaseVecT>::FaceHandle;

    boost::shared_ptr<lvr::MeshBuffer> apply(BaseMesh<BaseVecT>&& mesh);
};

} // namespace lvr2

#include <lvr2/algorithm/FinalizeAlgorithm.tcc>

#endif /* LVR2_ALGORITHM_FINALIZEALGORITHM_H_ */
