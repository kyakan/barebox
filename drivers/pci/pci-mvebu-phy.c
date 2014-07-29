/*
 * SoC specific PCIe PHY setup for Marvell MVEBU SoCs
 *
 * Sebastian Hesselbarth <sebastian.hesselbarth@gmail.com>
 *
 * based on Marvell BSP code (C) Marvell International Ltd.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <of.h>
#include <of_address.h>

#include "pci-mvebu.h"

static u32 mvebu_pcie_phy_indirect(void __iomem *phybase, u8 lane,
				   u8 off, u16 val, bool is_read)
{
	u32 reg = (lane << 24) | (off << 16) | val;

	if (is_read)
		reg |= BIT(31);
	writel(reg, phybase);

	return (is_read) ? readl(phybase) & 0xffff : 0;
}

static inline u32 mvebu_pcie_phy_read(void __iomem *phybase, u8 lane,
				      u8 off)
{
	return mvebu_pcie_phy_indirect(phybase, lane, off, 0, true);
}

static inline void mvebu_pcie_phy_write(void __iomem *phybase, u8 lane,
					u8 off, u16 val)
{
	mvebu_pcie_phy_indirect(phybase, lane, off, val, false);
}

#define ARMADA_370_PHY_OFFSET	0x1b00
#define ARMADA_370_SOC_CTRL	0x04
#define ARMADA_370_SERDES03_SEL	0x70

int armada_370_phy_setup(struct mvebu_pcie *pcie)
{
	struct device_node *np = of_find_compatible_node(NULL, NULL,
				 "marvell,armada-370-xp-system-controller");
	void __iomem *sysctrl = of_iomap(np, 0);
	void __iomem *phybase = pcie->base + ARMADA_370_PHY_OFFSET;
	u32 reg;

	if (!sysctrl)
		return -ENODEV;

	/* Enable PEX */
	reg = readl(sysctrl + ARMADA_370_SOC_CTRL);
	reg |= BIT(pcie->port);
	writel(reg, sysctrl + ARMADA_370_SOC_CTRL);

	/* Set SERDES selector */
	reg = readl(sysctrl + ARMADA_370_SERDES03_SEL);
	reg &= ~(0xf << (pcie->port * 4));
	reg |= (0x1 << (pcie->port * 4));
	writel(reg, sysctrl + ARMADA_370_SERDES03_SEL);

	/* BTS #232 - PCIe clock (undocumented) */
	writel(0x00000077, sysctrl + 0x2f0);

	/* Disable Link (undocumented) */
	reg = readl(pcie->base + 0x6c);
	reg &= ~0x3f0;
	reg |= BIT(4);
	writel(reg, pcie->base + 0x6c);

	/* PEX pipe configuration */
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xc1, 0x0025);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xc3, 0x000f);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xc8, 0x0005);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xd0, 0x0100);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xd1, 0x3014);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xc5, 0x011f);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x80, 0x1000);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x81, 0x0011);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x0f, 0x2a21);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x45, 0x00df);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x4f, 0x6219);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x01, 0xfc60);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x46, 0x0000);

	reg = mvebu_pcie_phy_read(phybase, pcie->lane, 0x48) & ~0x4;
	mvebu_pcie_phy_write(phybase, pcie->lane, 0x48, reg & 0xffff);

	mvebu_pcie_phy_write(phybase, pcie->lane, 0x02, 0x0040);
	mvebu_pcie_phy_write(phybase, pcie->lane, 0xc1, 0x0024);

	mdelay(15);

	return 0;
}
