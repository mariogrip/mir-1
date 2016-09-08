/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "src/platforms/mesa/server/x11/graphics/display.h"
#include "src/server/report/null/display_report.h"

#include "mir/graphics/display_configuration.h"

#include "mir/test/doubles/mock_egl.h"
#include "mir/test/doubles/mock_x11.h"
#include "mir/test/doubles/mock_gl_config.h"


namespace mg=mir::graphics;
namespace mgx=mg::X;
namespace mtd=mir::test::doubles;
namespace geom=mir::geometry;
using namespace testing;


namespace
{


class X11DisplayTest : public ::testing::Test
{
public:
    geom::Size size{1280, 1024};

    X11DisplayTest()
    {
        using namespace testing;
        EGLint const client_version = 2;

        ON_CALL(mock_egl, eglQueryString(_, EGL_EXTENSIONS))
            .WillByDefault(Return("other stuff and EGL_CHROMIUM_sync_control"));
                    
        ON_CALL(mock_egl, eglQueryContext(mock_egl.fake_egl_display,
                                          mock_egl.fake_egl_context,
                                          EGL_CONTEXT_CLIENT_VERSION,
                                          _))
            .WillByDefault(DoAll(SetArgPointee<3>(client_version),
                            Return(EGL_TRUE)));

        ON_CALL(mock_egl, eglQuerySurface(mock_egl.fake_egl_display,
                                          mock_egl.fake_egl_surface,
                                          EGL_WIDTH,
                                          _))
            .WillByDefault(DoAll(SetArgPointee<3>(size.width.as_int()),
                            Return(EGL_TRUE)));

        ON_CALL(mock_egl, eglQuerySurface(mock_egl.fake_egl_display,
                                          mock_egl.fake_egl_surface,
                                          EGL_HEIGHT,
                                          _))
            .WillByDefault(DoAll(SetArgPointee<3>(size.height.as_int()),
                            Return(EGL_TRUE)));

        ON_CALL(mock_egl, eglGetConfigAttrib(mock_egl.fake_egl_display,
                                             _,
                                             _,
                                             _))
            .WillByDefault(DoAll(SetArgPointee<3>(EGL_WINDOW_BIT),
                            Return(EGL_TRUE)));

        ON_CALL(mock_x11, XNextEvent(mock_x11.fake_x11.display,
                                     _))
            .WillByDefault(DoAll(SetArgPointee<1>(mock_x11.fake_x11.expose_event_return),
                       Return(1)));
    }

    void setup_x11_screen(geom::Size const& pixel, geom::Size const& mm, geom::Size const& window)
    {
        mock_x11.fake_x11.screen.width = pixel.width.as_int();
        mock_x11.fake_x11.screen.height = pixel.height.as_int();
        mock_x11.fake_x11.screen.mwidth = mm.width.as_int();
        mock_x11.fake_x11.screen.mheight = mm.height.as_int();
        size = window;
    }
    std::shared_ptr<mgx::Display> create_display()
    {
        return std::make_shared<mgx::Display>(
                   mock_x11.fake_x11.display,
                   size,
                   mock_gl_config,
                   std::make_shared<mir::report::null::DisplayReport>());
    }

    ::testing::NiceMock<mtd::MockEGL> mock_egl;
    ::testing::NiceMock<mtd::MockX11> mock_x11;
    mtd::MockGLConfig mock_gl_config;
};

}

TEST_F(X11DisplayTest, creates_display_successfully)
{
    using namespace testing;

    EXPECT_CALL(mock_egl, eglGetDisplay(mock_x11.fake_x11.display))
        .Times(Exactly(1));

    EXPECT_CALL(mock_x11, XCreateWindow_wrapper(mock_x11.fake_x11.display,_, size.width.as_int(), size.height.as_int(),_,_,_,_,_,_))
        .Times(Exactly(1));

    EXPECT_CALL(mock_egl, eglCreateContext(mock_egl.fake_egl_display,_, EGL_NO_CONTEXT,_))
        .Times(Exactly(1));

    EXPECT_CALL(mock_egl, eglCreateWindowSurface(mock_egl.fake_egl_display,_, reinterpret_cast<mtd::MockEGL::AnyNativeType>(mock_x11.fake_x11.window), nullptr))
        .Times(Exactly(1));

    EXPECT_CALL(mock_x11, XNextEvent(mock_x11.fake_x11.display,_))
        .Times(AtLeast(1));

    EXPECT_CALL(mock_x11, XMapWindow(mock_x11.fake_x11.display,_))
        .Times(Exactly(1));

    auto display = create_display();
}

TEST_F(X11DisplayTest, respects_gl_config)
{
    EGLint const depth_bits{24};
    EGLint const stencil_bits{8};

    EXPECT_CALL(mock_gl_config, depth_buffer_bits())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(depth_bits));
    EXPECT_CALL(mock_gl_config, stencil_buffer_bits())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(stencil_bits));

    EXPECT_CALL(mock_egl,
                eglChooseConfig(
                    _,
                    AllOf(mtd::EGLConfigContainsAttrib(EGL_DEPTH_SIZE, depth_bits),
                          mtd::EGLConfigContainsAttrib(EGL_STENCIL_SIZE, stencil_bits)),
                    _,_,_))
        .Times(AtLeast(1));

    auto display = create_display();
}

TEST_F(X11DisplayTest, calculates_physical_size_of_display_based_on_default_screen)
{
    auto const pixel = geom::Size{2560, 1080};
    auto const mm = geom::Size{677, 290};
    auto const window = geom::Size{1280, 1024};
    auto const pixel_width = float(mm.width.as_int()) / float(pixel.width.as_int());
    auto const pixel_height = float(mm.height.as_int()) / float(pixel.height.as_int());
    auto const expected_size = geom::Size{window.width * pixel_width, window.height * pixel_height};

    setup_x11_screen(pixel, mm, window);

    auto display = create_display();
    auto config = display->configuration();
    geom::Size reported_size;
    config->for_each_output(
        [&reported_size](mg::DisplayConfigurationOutput const& output)
        {
            reported_size = output.physical_size_mm;
        }
        );

    EXPECT_THAT(reported_size, Eq(expected_size));
}

TEST_F(X11DisplayTest, reports_a_resolution_that_matches_the_window_size)
{
    auto const pixel = geom::Size{2560, 1080};
    auto const mm = geom::Size{677, 290};
    auto const window = geom::Size{1280, 1024};

    setup_x11_screen(pixel, mm, window);

    auto display = create_display();
    auto config = display->configuration();
    geom::Size reported_resolution;
    config->for_each_output(
        [&reported_resolution](mg::DisplayConfigurationOutput const& output)
        {
            reported_resolution = output.modes[0].size;
        }
        );

    EXPECT_THAT(reported_resolution, Eq(window));
}
