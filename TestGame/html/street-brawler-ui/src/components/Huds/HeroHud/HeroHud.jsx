import { HealthBar } from '../../HealthBar/HealthBar';

export const HeroHud = ({ picture, title, hp }) => {
  return (
    <div className="character-hud hero">
      <div className="unit-display hero">
        <img src={picture} className="portrait hero" alt={title} />
        <div className="bars-container">
          <span className="bars-title hero">{title}</span>
          <HealthBar hp={hp} type="main" />
        </div>
      </div>
    </div>
  );
}
