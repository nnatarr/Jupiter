-- VHDL automatically generated from
-- [2band2_or2 x5 x6 [2band2_or2 x0 -x5 x0 -x3] [2band2_or2 [2band2_or2 x4 0 0 x7] [2band2_or2 -x0 0 0 x2] -x4 [2band2_or2 -x0 0 0 x1]]]

--------------------- Cell myCell ----------------------
entity myCell is port(x0, x1, x2, x3, x4, x5, x6, x7: in BIT; y: out BIT);
  end myCell;

architecture myCell_BODY of myCell is
  component 2band2_or2 port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;

  signal inv_0, inv_1, inv_2, inv_3, net_0, net_1, net_2, net_3, net_4: BIT;

begin
  ELM5: 2band2_or2 port map(x5, x6, net_0, net_4, y);
  ELM4: 2band2_or2 port map(net_1, net_2, inv_2, net_3, net_4);
  ELM3: 2band2_or2 port map(inv_0, 0, 0, x1, net_3);
  ELM2: 2band2_or2 port map(inv_0, 0, 0, x2, net_2);
  ELM1: 2band2_or2 port map(x4, 0, 0, x7, net_1);
  ELM0: 2band2_or2 port map(x0, inv_3, x0, inv_1, net_0);
  PSEUDO_INVERT3: inverter port map(x5, inv_3);
  PSEUDO_INVERT2: inverter port map(x4, inv_2);
  PSEUDO_INVERT1: inverter port map(x3, inv_1);
  PSEUDO_INVERT0: inverter port map(x0, inv_0);
end myCell_BODY;
