
#include <linux/rk_fb.h>
#include <linux/device.h>
#include "lcd.h"
#include "../hdmi/rk_hdmi.h"

static struct rk_screen *rk_screen;
int  rk_fb_get_prmry_screen(struct rk_screen *screen)
{
	memcpy(screen, rk_screen, sizeof(struct rk_screen));
	return 0;
}

int rk_fb_set_prmry_screen(struct rk_screen *screen)
{
	rk_screen->lcdc_id = screen->lcdc_id;
	rk_screen->screen_id = screen->screen_id;
	rk_screen->x_mirror = screen->x_mirror;
	rk_screen->y_mirror = screen->y_mirror;
	rk_screen->overscan.left = screen->overscan.left;
	rk_screen->overscan.top = screen->overscan.left;
	rk_screen->overscan.right = screen->overscan.left;
	rk_screen->overscan.bottom = screen->overscan.left;
	return 0;
}

size_t get_fb_size(void)
{
	size_t size = 0;
	int xres = rk_screen->mode.xres;
	int yres = rk_screen->mode.yres;

	/*align as 64 bytes(16*4) in an odd number of times*/
	xres = ALIGN_64BYTE_ODD_TIMES(xres, ALIGN_PIXEL_64BYTE_RGB8888);

	#if defined(CONFIG_THREE_FB_BUFFER)
		size = (xres * yres << 2) * 3; //three buffer
	#else
		size = (xres * yres << 2) << 1; //two buffer
	#endif
	return ALIGN(size, SZ_1M);
}

static int rk_screen_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	int ret;

	if (!np) {
		dev_err(&pdev->dev, "Missing device tree node.\n");
		return -EINVAL;
	}
	rk_screen = devm_kzalloc(&pdev->dev,
			sizeof(struct rk_screen), GFP_KERNEL);
	if (!rk_screen) {
		dev_err(&pdev->dev, "kmalloc for rk screen fail!");
		return  -ENOMEM;
	}
	ret = rk_fb_prase_timing_dt(np, rk_screen);
	dev_info(&pdev->dev, "rockchip screen probe %s\n",
				ret ? "failed" : "success");
	return ret;
}

static const struct of_device_id rk_screen_dt_ids[] = {
	{ .compatible = "rockchip,screen", },
	{}
};

static struct platform_driver rk_screen_driver = {
	.probe		= rk_screen_probe,
	.driver		= {
		.name	= "rk-screen",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rk_screen_dt_ids),
	},
};

static int __init rk_screen_init(void)
{
	return platform_driver_register(&rk_screen_driver);
}

static void __exit rk_screen_exit(void)
{
	platform_driver_unregister(&rk_screen_driver);
}

fs_initcall(rk_screen_init);
module_exit(rk_screen_exit);

