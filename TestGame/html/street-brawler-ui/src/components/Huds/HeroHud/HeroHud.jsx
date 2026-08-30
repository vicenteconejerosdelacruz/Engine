import { HealthBar } from '../../HealthBar/HealthBar';
import './HeroHud.css';

export const HeroHud = ({ picture, title, lives=1, hp, maxHp }) => {
  return (
    <div className="character-hud hero">
      <div className="unit-display hero">
        <span className="hero-lives">{lives}</span>
        <img src={picture} className="portrait hero" alt={title} />
        <div className="bars-container">
          <span className="bars-title hero">{title}</span>
          <HealthBar hp={hp} maxHp={maxHp} type="main" />
        </div>
      </div>
    </div>
  );
}
