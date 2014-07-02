entity lut1 is
    port (
        input: in std_ulogic_vector(2 downto 0);
        output: out BIT;
    );
end entity;

architecture lut1_arch of lut1 is
begin
    process (input) is
    begin
        case input of
            when "000" => output <= "1";
            when "001" => output <= "1";
            when "010" => output <= "1";
            when "011" => output <= "1";
            when "100" => output <= "1";
            when "101" => output <= "1";
            when "110" => output <= "1";
            when "111" => output <= "0";
        end case;
    end process;
end architecture lut1_arch;

entity lut0 is
    port (
        input: in std_ulogic_vector(2 downto 0);
        output: out BIT;
    );
end entity;

architecture lut0_arch of lut0 is
begin
    process (input) is
    begin
        case input of
            when "000" => output <= "0";
            when "001" => output <= "1";
            when "010" => output <= "1";
            when "011" => output <= "1";
            when "100" => output <= "1";
            when "101" => output <= "1";
            when "110" => output <= "1";
            when "111" => output <= "1";
        end case;
    end process;
end architecture lut0_arch;


entity main_entity is
    port (
        x0, x1, x2: in BIT;
        output: out BIT;
    );
end entity;

architecture main_entity_arch of main_entity is
    component lut1 port(tx0, tx1, tx2: in BIT; txout: out BIT);
    end component;
    component lut0 port(tx0, tx1, tx2: in BIT; txout: out BIT);
    end component;

    signal s0: BIT;
begin
    lut1 port map(x0, x1, x2, s0);
    lut0 port map(0, 0, s0, output);
end architecture main_entity_arch;